#include "Bridge/ControlServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#define RAPIDJSON_HAS_STDSTRING 1
#include "ThirdParty/rapidjson/document.h"
#include "ThirdParty/rapidjson/stringbuffer.h"
#include "ThirdParty/rapidjson/writer.h"

#include "RuiNodeEditor/RuiNodeEditor.h"
#include "RuiRendering/RenderManager.h"
#include "RenderFrameworks/RenderFramework.h"
#include "Settings.h"
#include "ImageAtlas.h"
#undef GetObject

namespace fs = std::filesystem;

namespace SereBridge
{
namespace
{
	struct Request
	{
		std::string method;
		std::string params;          // raw JSON object
		std::string response;        // raw JSON object, filled on the main thread
		bool done = false;
	};

	Context           g_ctx;
	std::atomic<bool> g_running{ false };
	unsigned short    g_port = 0;
	std::thread       g_thread;
	SOCKET            g_listen = INVALID_SOCKET;

	std::mutex                    g_mutex;
	std::condition_variable       g_cv;
	std::deque<Request*>          g_queue;

	std::string JsonEscape(const std::string& in)
	{
		rapidjson::StringBuffer buf;
		rapidjson::Writer<rapidjson::StringBuffer> w(buf);
		w.String(in);
		return std::string(buf.GetString(), buf.GetSize());
	}

	std::string Fail(const std::string& message)
	{
		return "{\"ok\":false,\"error\":" + JsonEscape(message) + "}";
	}

	std::string OkRaw(const std::string& body)
	{
		return "{\"ok\":true," + body + "}";
	}

	const char* kBase64 =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string Base64(const std::vector<uint8_t>& data)
	{
		std::string out;
		out.reserve((data.size() + 2) / 3 * 4);
		size_t i = 0;
		for (; i + 2 < data.size(); i += 3) {
			uint32_t v = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
			out += kBase64[(v >> 18) & 63];
			out += kBase64[(v >> 12) & 63];
			out += kBase64[(v >> 6) & 63];
			out += kBase64[v & 63];
		}
		if (i + 1 == data.size()) {
			uint32_t v = data[i] << 16;
			out += kBase64[(v >> 18) & 63];
			out += kBase64[(v >> 12) & 63];
			out += "==";
		} else if (i + 2 == data.size()) {
			uint32_t v = (data[i] << 16) | (data[i + 1] << 8);
			out += kBase64[(v >> 18) & 63];
			out += kBase64[(v >> 12) & 63];
			out += kBase64[(v >> 6) & 63];
			out += '=';
		}
		return out;
	}

	// ------------------------------------------------------------------
	// Handlers. Every one of these runs on the main thread from Pump().
	// ------------------------------------------------------------------
	std::string Handle(const std::string& method, rapidjson::Document& params)
	{
		auto str = [&](const char* key, const char* fallback = "") -> std::string {
			if (params.IsObject() && params.HasMember(key) && params[key].IsString())
				return params[key].GetString();
			return fallback;
		};

		if (method == "health") {
			return OkRaw("\"editor\":" + std::string(g_ctx.editor ? "true" : "false") +
				",\"nodes\":" + std::to_string(g_ctx.editor ? g_ctx.editor->NodeCount() : 0) +
				",\"file\":" + JsonEscape(g_ctx.editor ? g_ctx.editor->CurrentFilePath() : ""));
		}

		if (!g_ctx.editor)
			return Fail("editor not attached");

		if (method == "graph.get") {
			return OkRaw("\"graph\":" + g_ctx.editor->SerializeToString());
		}

		if (method == "graph.set") {
			if (!params.IsObject() || !params.HasMember("graph"))
				return Fail("graph.set needs a 'graph' object");
			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> w(buf);
			params["graph"].Accept(w);
			std::string error;
			if (!g_ctx.editor->DeserializeFromString(std::string(buf.GetString(), buf.GetSize()), error))
				return Fail(error);
			return OkRaw("\"nodes\":" + std::to_string(g_ctx.editor->NodeCount()));
		}

		if (method == "graph.load") {
			std::string path = str("path");
			if (path.empty() || !fs::exists(path))
				return Fail("no such file: " + path);
			g_ctx.editor->LoadFromPath(path);
			return OkRaw("\"nodes\":" + std::to_string(g_ctx.editor->NodeCount()));
		}

		if (method == "graph.save") {
			std::string path = str("path", g_ctx.editor->CurrentFilePath().c_str());
			if (path.empty())
				return Fail("no path given and the graph has never been saved");
			g_ctx.editor->SaveToPath(path);
			return OkRaw("\"path\":" + JsonEscape(path));
		}

		if (method == "args.set") {
			if (!g_ctx.render)
				return Fail("render instance not attached");
			if (!params.IsObject() || !params.HasMember("args") || !params["args"].IsObject())
				return Fail("args.set needs an 'args' object");
			int count = 0;
			for (auto it = params["args"].MemberBegin(); it != params["args"].MemberEnd(); ++it) {
				std::string name = it->name.GetString();
				const rapidjson::Value& v = it->value;
				if (v.IsString())      g_ctx.render->arguments[name] = std::string(v.GetString());
				else if (v.IsBool())   g_ctx.render->arguments[name] = v.GetBool();
				else if (v.IsInt())    g_ctx.render->arguments[name] = v.GetInt();
				else if (v.IsNumber()) g_ctx.render->arguments[name] = (float)v.GetDouble();
				else continue;
				count++;
			}
			return OkRaw("\"set\":" + std::to_string(count));
		}

		if (method == "args.get") {
			if (!g_ctx.render)
				return Fail("render instance not attached");
			std::string body = "\"args\":{";
			bool first = true;
			for (auto& [name, value] : g_ctx.render->arguments) {
				if (!first) body += ",";
				first = false;
				body += JsonEscape(name) + ":";
				if (value.type() == typeid(std::string))    body += JsonEscape(std::any_cast<std::string>(value));
				else if (value.type() == typeid(bool))      body += std::any_cast<bool>(value) ? "true" : "false";
				else if (value.type() == typeid(int))       body += std::to_string(std::any_cast<int>(value));
				else if (value.type() == typeid(float))     body += std::to_string(std::any_cast<float>(value));
				else                                       body += "null";
			}
			body += "}";
			return OkRaw(body);
		}

		if (method == "preview.png") {
			if (!g_renderFramework)
				return Fail("no render framework");
			std::vector<uint8_t> png;
			int w = 0, h = 0;
			if (!g_renderFramework->CapturePreviewPng(png, w, h))
				return Fail("preview capture failed");
			std::string path = str("path");
			if (!path.empty()) {
				std::ofstream f(path, std::ios::binary);
				f.write((const char*)png.data(), png.size());
				f.close();
				return OkRaw("\"width\":" + std::to_string(w) + ",\"height\":" + std::to_string(h) +
					",\"path\":" + JsonEscape(path));
			}
			return OkRaw("\"width\":" + std::to_string(w) + ",\"height\":" + std::to_string(h) +
				",\"png_base64\":\"" + Base64(png) + "\"");
		}

		if (method == "export") {
			std::string path = str("path");
			if (path.empty()) {
				std::string name = str("name");
				if (name.empty())
					return Fail("export needs 'path' or 'name'");
				fs::path dir = g_ctx.editor->CurrentFilePath().empty()
					? fs::current_path()
					: fs::path(g_ctx.editor->CurrentFilePath()).parent_path();
				path = (dir / (name + ".ruip")).string();
			}
			std::string message = g_ctx.editor->ExportToPath(path);
			return OkRaw("\"message\":" + JsonEscape(message));
		}

		if (method == "assets.find") {
			std::string needle = str("query");
			int limit = 200;
			if (params.IsObject() && params.HasMember("limit") && params["limit"].IsInt())
				limit = params["limit"].GetInt();

			std::string lowerNeedle = needle;
			for (auto& c : lowerNeedle) c = (char)tolower((unsigned char)c);

			std::string body = "\"total\":" + std::to_string(imageAssetMap.size()) + ",\"matches\":[";
			int found = 0;
			for (auto& [guid, asset] : imageAssetMap) {
				if (asset.name.empty()) continue;
				if (!lowerNeedle.empty()) {
					std::string lowerName = asset.name;
					for (auto& c : lowerName) c = (char)tolower((unsigned char)c);
					if (lowerName.find(lowerNeedle) == std::string::npos) continue;
				}
				if (found >= limit) break;
				if (found) body += ",";
				body += JsonEscape(asset.name);
				found++;
			}
			body += "],\"shown\":" + std::to_string(found);
			return OkRaw(body);
		}

		if (method == "settings.get") {
			if (!g_ctx.settings)
				return Fail("settings not attached");
			auto size = g_ctx.settings->GetRuiSize();
			return OkRaw("\"gamePath\":" + JsonEscape(g_ctx.settings->GetTitanfall2Path()) +
				",\"repakExe\":" + JsonEscape(g_ctx.settings->GetRepakExePath()) +
				",\"autoDeploy\":" + std::string(g_ctx.settings->GetAutoDeploy() ? "true" : "false") +
				",\"width\":" + std::to_string(size.width) +
				",\"height\":" + std::to_string(size.height));
		}

		return Fail("unknown method: " + method);
	}

	// ------------------------------------------------------------------
	// Socket thread: minimal HTTP/1.1, POST /rpc only.
	// ------------------------------------------------------------------
	bool RecvAll(SOCKET s, std::string& out, size_t want)
	{
		char buf[8192];
		while (out.size() < want) {
			int n = recv(s, buf, (int)sizeof(buf), 0);
			if (n <= 0) return false;
			out.append(buf, n);
		}
		return true;
	}

	void SendResponse(SOCKET s, const std::string& body, int status = 200)
	{
		std::string head = "HTTP/1.1 " + std::to_string(status) +
			(status == 200 ? " OK" : " Bad Request") + "\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: close\r\n\r\n";
		send(s, head.c_str(), (int)head.size(), 0);
		if (!body.empty())
			send(s, body.c_str(), (int)body.size(), 0);
	}

	void ServeOne(SOCKET client)
	{
		std::string raw;
		size_t headerEnd = std::string::npos;
		char buf[8192];
		while (true) {
			int n = recv(client, buf, (int)sizeof(buf), 0);
			if (n <= 0) { closesocket(client); return; }
			raw.append(buf, n);
			headerEnd = raw.find("\r\n\r\n");
			if (headerEnd != std::string::npos) break;
			if (raw.size() > (1u << 20)) { closesocket(client); return; }
		}

		size_t contentLength = 0;
		{
			std::string headers = raw.substr(0, headerEnd);
			std::string lower = headers;
			for (auto& c : lower) c = (char)tolower((unsigned char)c);
			size_t at = lower.find("content-length:");
			if (at != std::string::npos)
				contentLength = strtoull(headers.c_str() + at + 15, nullptr, 10);
		}

		std::string body = raw.substr(headerEnd + 4);
		if (body.size() < contentLength && !RecvAll(client, body, contentLength)) {
			closesocket(client);
			return;
		}
		body.resize(contentLength);

		rapidjson::Document doc;
		doc.Parse(body.c_str(), body.size());
		if (doc.HasParseError() || !doc.IsObject() ||
			!doc.HasMember("method") || !doc["method"].IsString()) {
			SendResponse(client, Fail("expected {\"method\":...,\"params\":{...}}"), 400);
			closesocket(client);
			return;
		}

		Request req;
		req.method = doc["method"].GetString();
		if (doc.HasMember("params")) {
			rapidjson::StringBuffer sb;
			rapidjson::Writer<rapidjson::StringBuffer> w(sb);
			doc["params"].Accept(w);
			req.params.assign(sb.GetString(), sb.GetSize());
		} else {
			req.params = "{}";
		}

		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_queue.push_back(&req);
		}
		{
			std::unique_lock<std::mutex> lock(g_mutex);
			// A frame that never comes must not wedge the socket thread.
			g_cv.wait_for(lock, std::chrono::seconds(180), [&] { return req.done; });
			if (!req.done) {
				for (auto it = g_queue.begin(); it != g_queue.end(); ++it) {
					if (*it == &req) { g_queue.erase(it); break; }
				}
				req.response = Fail("timed out waiting for the editor main thread");
			}
		}

		SendResponse(client, req.response);
		closesocket(client);
	}

	void ThreadMain()
	{
		while (g_running) {
			sockaddr_in addr{};
			int len = sizeof(addr);
			SOCKET client = accept(g_listen, (sockaddr*)&addr, &len);
			if (client == INVALID_SOCKET) {
				if (!g_running) break;
				continue;
			}
			ServeOne(client);
		}
	}
} // namespace

bool Start(unsigned short port, const Context& ctx)
{
	if (g_running) return true;
	g_ctx = ctx;

	WSADATA wsa{};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[SERE-BRIDGE] WSAStartup failed\n");
		return false;
	}

	g_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listen == INVALID_SOCKET) {
		printf("[SERE-BRIDGE] socket() failed\n");
		return false;
	}

	int yes = 1;
	setsockopt(g_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	// Loopback only. This channel executes local builds and writes into the
	// game install; it must never be reachable off-box.
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (bind(g_listen, (sockaddr*)&addr, sizeof(addr)) != 0 ||
		listen(g_listen, 8) != 0) {
		printf("[SERE-BRIDGE] bind/listen on 127.0.0.1:%u failed\n", port);
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
		return false;
	}

	g_port = port;
	g_running = true;
	g_thread = std::thread(ThreadMain);
	printf("[SERE-BRIDGE] listening on 127.0.0.1:%u\n", port);
	fflush(stdout);
	return true;
}

void Stop()
{
	if (!g_running) return;
	g_running = false;
	if (g_listen != INVALID_SOCKET) {
		closesocket(g_listen);
		g_listen = INVALID_SOCKET;
	}
	g_cv.notify_all();
	if (g_thread.joinable()) g_thread.join();
	WSACleanup();
}

void Pump()
{
	while (true) {
		Request* req = nullptr;
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (g_queue.empty()) return;
			req = g_queue.front();
			g_queue.pop_front();
		}

		rapidjson::Document params;
		params.Parse(req->params.c_str(), req->params.size());

		try {
			req->response = Handle(req->method, params);
		} catch (const std::exception& e) {
			req->response = Fail(std::string("handler threw: ") + e.what());
		} catch (...) {
			req->response = Fail("handler threw (unknown)");
		}

		{
			std::lock_guard<std::mutex> lock(g_mutex);
			req->done = true;
		}
		g_cv.notify_all();
	}
}

unsigned short BoundPort() { return g_port; }

} // namespace SereBridge
