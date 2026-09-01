#pragma once

#include <string>
#include <functional>

class NodeEditor;
class RenderInstance;
class Settings;

// Loopback JSON-RPC control channel so external tooling (the SERE MCP server)
// can drive the editor: mutate the graph, set preview arguments, capture the
// RUI render target, and export. Requests are queued on the socket thread and
// executed on the main thread, because ImGui, D3D and the node graph are all
// single-threaded.
namespace SereBridge
{
	struct Context
	{
		NodeEditor*     editor   = nullptr;
		RenderInstance* render   = nullptr;
		Settings*       settings = nullptr;
	};

	bool Start(unsigned short port, const Context& ctx);
	void Stop();

	// Drain queued requests. Call once per frame from the main loop.
	void Pump();

	unsigned short BoundPort();
}
