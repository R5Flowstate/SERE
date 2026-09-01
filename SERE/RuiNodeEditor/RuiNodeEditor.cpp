#include "RuiNodeEditor.h"
#include "imgui/imgui.h"

#include "imgui/imgui_stdlib.h"

#include "Util.h"
#include <fstream>

#include "Nodes/ArgumentNodes.h"
#include "Nodes/TransformNodes.h"
#include "Nodes/RenderJobNodes.h"
#include "Nodes/ConstantVarNodes.h"
#include "Nodes/SplitMergeNodes.h"
#include "Nodes/MathNodes.h"
#include "Nodes/GlobalNodes.h"
#include "RenderFrameworks/RenderFramework.h"
#include "Nodes/ConditionalNodes.h"

#include "Thirdparty/rapidjson/istreamwrapper.h"
#include "ThirdParty/rapidjson/prettywriter.h"
#include "ThirdParty/rapidjson/stringbuffer.h"
#include "ThirdParty/nativefiledialog-extended/src/include/nfd.hpp"
#undef GetObject

NodeEditor::NodeEditor(RenderInstance& rend):render(rend) {
	mINF.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
		RightClickPopup(node);
	});
	mINF.droppedLinkPopUpContent([this](ImFlow::Pin* pin) {
		LinkDroppedPopup(pin);
	});
	mINF.setOnModified([this]() { MarkDirty(); });
	ImFlow::StyleManager& styles = mINF.getStyleManager();;
	SetStyles(styles);

}

std::string NodeEditor::GetWindowTitle() const {
	std::string title = "SERE";
	if (!m_currentFilePath.empty())
		title += " - " + fs::path(m_currentFilePath).filename().string();
	else
		title += " - Untitled";
	if (m_dirty)
		title += " *";
	return title;
}



void NodeEditor::Draw() {
	std::string title = GetWindowTitle() + "###NodeEditor";
	ImGui::Begin(title.c_str());

	mINF.update();

	ImGui::End();
}

void NodeEditor::Clear() {
	mINF.clearAll();
}

std::string NodeEditor::SerializeToString() {
	if (!mINF.getNodesCount()) return "{\"Nodes\":[],\"Links\":[]}";

	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::GenericValue<rapidjson::UTF8<>> nodeArray;
	nodeArray.SetArray();
	for (auto& [nodeId, nodePtr] : mINF.getNodes()) {
		rapidjson::GenericValue<rapidjson::UTF8<>> val;
		val.SetObject();
		std::dynamic_pointer_cast<RuiBaseNode>(nodePtr)->Serialize(val, doc.GetAllocator());
		nodeArray.PushBack(val, doc.GetAllocator());
	}
	doc.AddMember("Nodes", nodeArray, doc.GetAllocator());
	rapidjson::GenericValue<rapidjson::UTF8<>> linkArray;
	linkArray.SetArray();
	for (auto& link : mINF.getLinks()) {
		rapidjson::GenericValue<rapidjson::UTF8<>> val;
		val.SetObject();
		auto lnk = link.lock();
		val.AddMember("LeftNode", lnk->left()->getParent()->getUID(), doc.GetAllocator());
		val.AddMember("LeftPin", lnk->left()->getName(), doc.GetAllocator());
		val.AddMember("RightNode", lnk->right()->getParent()->getUID(), doc.GetAllocator());
		val.AddMember("RightPin", lnk->right()->getName(), doc.GetAllocator());
		linkArray.PushBack(val, doc.GetAllocator());
	}
	doc.AddMember("Links", linkArray, doc.GetAllocator());
	// The element size is baked into the asset, so it belongs to the graph, not
	// to whatever the editor happens to be set to.
	doc.AddMember("RuiWidth", render.elementWidth, doc.GetAllocator());
	doc.AddMember("RuiHeight", render.elementHeight, doc.GetAllocator());

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return std::string(buffer.GetString(), buffer.GetSize());
}

void NodeEditor::SerializeToPath(const fs::path& path) {
	std::string json = SerializeToString();
	std::ofstream outFile{path};
	outFile.write(json.data(), json.size());
	outFile.close();
}

void NodeEditor::DeserializeFromPath(const fs::path& path) {
	printf("[SERE] Open graph: %s\n", path.string().c_str()); fflush(stdout);
	std::ifstream file(path);
	if (!file.is_open()) {
		printf("Error Opening JSON File %s\n", path.string().c_str());
		return;
	}
	rapidjson::IStreamWrapper wrap(file);
	rapidjson::Document doc;
	doc.ParseStream(wrap);
	if (doc.HasParseError()) {
		printf("Error in JSON File %s\n", path.string().c_str());
		return;
	}
	DeserializeDocument(doc);
}

void NodeEditor::DeserializeDocument(rapidjson::Document& doc) {
	Clear();

	rapidjson::GenericObject root = doc.GetObject();
	if (!(root.HasMember("Nodes") && root["Nodes"].IsArray())) return;
	if (!(root.HasMember("Links") && root["Links"].IsArray())) return;

	if (root.HasMember("RuiWidth") && root["RuiWidth"].IsNumber() &&
		root.HasMember("RuiHeight") && root["RuiHeight"].IsNumber()) {
		const float w = root["RuiWidth"].GetFloat();
		const float h = root["RuiHeight"].GetFloat();
		if (w > 0.f && h > 0.f && (w != render.elementWidth || h != render.elementHeight)) {
			render.SetSize(w, h);
			g_renderFramework->RuiReCreatePipeline((int)w, (int)h);
			printf("[SERE] graph canvas %gx%g\n", w, h); fflush(stdout);
		}
	}
	rapidjson::GenericArray nodes = root["Nodes"].GetArray();
	int nodeCount = 0;
	for (auto itr = nodes.Begin(); itr != nodes.End(); itr++) {
		if (!itr->IsObject()) continue;
		rapidjson::GenericObject node = itr->GetObject();
		if (!(node.HasMember("Name") && node["Name"].IsString())) continue;
		if (!(node.HasMember("Category") && node["Category"].IsString())) continue;
		std::string name = node["Name"].GetString();
		std::string category = node["Category"].GetString();
		if (!nodeTypes.contains(category)) { printf("Unknown Category %s\n", category.c_str()); continue; }
		if (!nodeTypes[category].contains(name)) { printf("Unknown Node %s\n", name.c_str()); continue; }
		try {
			nodeTypes[category][name].RecreateNode(mINF, render, mINF.getStyleManager(), node);
			nodeCount++;
		} catch (const std::exception& e) {
			printf("[SERE] Node recreate failed %s/%s: %s\n", category.c_str(), name.c_str(), e.what());
		} catch (...) {
			printf("[SERE] Node recreate failed %s/%s (unknown)\n", category.c_str(), name.c_str());
		}
	}
	rapidjson::GenericArray links = root["Links"].GetArray();
	int linkCount = 0;
	for (auto itr = links.Begin(); itr != links.End(); itr++) {
		if (!itr->IsObject()) continue;
		rapidjson::GenericObject link = itr->GetObject();
		if (!(link.HasMember("LeftNode") && link["LeftNode"].IsUint64())) continue;
		if (!(link.HasMember("LeftPin") && link["LeftPin"].IsString())) continue;
		if (!(link.HasMember("RightNode") && link["RightNode"].IsUint64())) continue;
		if (!(link.HasMember("RightPin") && link["RightPin"].IsString())) continue;
		uint64_t leftId = link["LeftNode"].GetUint64();
		uint64_t rightId = link["RightNode"].GetUint64();
		std::string leftPinName = link["LeftPin"].GetString();
		std::string rightPinName = link["RightPin"].GetString();
		if (!mINF.getNodes().contains(leftId)) continue;
		if (!mINF.getNodes().contains(rightId)) continue;
		auto* leftPin = mINF.getNodes()[leftId]->outPin(leftPinName);
		auto* rightPin = mINF.getNodes()[rightId]->inPin(rightPinName);
		if (!leftPin || !rightPin) {
			printf("[SERE] Bad link pin L=%s R=%s\n", leftPinName.c_str(), rightPinName.c_str());
			continue;
		}
		leftPin->createLink(rightPin);
		linkCount++;
	}
	printf("[SERE] Open OK: nodes=%d links=%d\n", nodeCount, linkCount); fflush(stdout);
}

void NodeEditor::Save() {
	if (m_currentFilePath.empty()) { SaveAs(); return; }
	SerializeToPath(m_currentFilePath);
	m_dirty = false;
}

void NodeEditor::SaveAs() {
	NFD::Guard nfdGuard;
	nfdfilteritem_t filter("Graph", "json");
	NFD::UniquePath nfdPath;
	if (NFD::SaveDialog(nfdPath, &filter, 1) != NFD_OKAY) return;
	m_currentFilePath = nfdPath.get();
	SerializeToPath(m_currentFilePath);
	m_dirty = false;
}

void NodeEditor::Load() {
	auto doLoad = [this]() {
		NFD::Guard nfdGuard;
		nfdfilteritem_t filter("Graph", "json");
		NFD::UniquePath nfdPath;
		if (NFD::OpenDialog(nfdPath, &filter, 1) != NFD_OKAY) return;
		m_currentFilePath = nfdPath.get();
		DeserializeFromPath(m_currentFilePath);
		m_dirty = false;
	};
	if (m_dirty) {
		m_pendingAction = doLoad;
		m_showUnsavedPrompt = true;
	} else {
		doLoad();
	}
}

void NodeEditor::New() {
	auto doNew = [this]() {
		Clear();
		m_currentFilePath.clear();
		m_dirty = false;
	};
	if (m_dirty) {
		m_pendingAction = doNew;
		m_showUnsavedPrompt = true;
	} else {
		doNew();
	}
}

void NodeEditor::DrawUnsavedPrompt() {
	if (m_showUnsavedPrompt) {
		ImGui::OpenPopup("Unsaved Changes");
		m_showUnsavedPrompt = false;
	}
	if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("You have unsaved changes. Save before continuing?");
		ImGui::Spacing();
		if (ImGui::Button("Save", ImVec2(100, 0))) {
			Save();
			if (m_pendingAction) m_pendingAction();
			m_pendingAction = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Don't Save", ImVec2(100, 0))) {
			m_dirty = false;
			if (m_pendingAction) m_pendingAction();
			m_pendingAction = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0))) {
			m_pendingAction = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void NodeEditor::Export() {

	nfdfilteritem_t filter("RuiPackage","ruip");
	NFD::UniquePath nfdPath;
	if(NFD::SaveDialog(nfdPath,&filter,1) != NFD_OKAY) return;
	fs::path path (nfdPath.get());
	std::string name = path.filename().replace_extension("").string();
	RuiExportPrototype proto(render,name);
	proto.Generate(mINF.getNodes(),render);
	NFD::Guard nfdGuard;

	proto.WriteToFile(path);

	fs::path cppPath = path;
	cppPath.replace_extension("cpp");
	fs::path headerPath = cppPath.parent_path() / "RuiHeaders.h";
	exportPopupMessage = "Exported:\n  " + path.string() + "\n  " + cppPath.string() + "\n  " + headerPath.string();

	if (settings && settings->GetAutoDeploy())
		BuildAndDeploy(path.parent_path(), name);

	showExportPopup = true;
}

bool NodeEditor::DeserializeFromString(const std::string& json, std::string& error) {
	rapidjson::Document doc;
	doc.Parse(json.c_str(), json.size());
	if (doc.HasParseError()) {
		error = "JSON parse error at offset " + std::to_string(doc.GetErrorOffset());
		return false;
	}
	if (!doc.IsObject()) {
		error = "root is not an object";
		return false;
	}
	DeserializeDocument(doc);
	m_dirty = true;
	return true;
}

void NodeEditor::LoadFromPath(const fs::path& path) {
	m_currentFilePath = path.string();
	DeserializeFromPath(path);
	m_dirty = false;
}

void NodeEditor::SaveToPath(const fs::path& path) {
	m_currentFilePath = path.string();
	SerializeToPath(path);
	m_dirty = false;
}

std::string NodeEditor::ExportToPath(const fs::path& path) {
	std::string name = fs::path(path).filename().replace_extension("").string();
	RuiExportPrototype proto(render, name);
	proto.Generate(mINF.getNodes(), render);
	proto.WriteToFile(path);

	fs::path cppPath = path;
	cppPath.replace_extension("cpp");
	std::string message = "Exported: " + path.string() + " | " + cppPath.string();

	if (settings && settings->GetAutoDeploy()) {
		BuildAndDeploy(path.parent_path(), name);
		message += " | deployed " + name + ".rpak";
	}
	return message;
}

void NodeEditor::BuildAndDeploy(const fs::path& exportDir, const std::string& name)
{
	if (!settings) return;

	std::string gamePath = settings->GetTitanfall2Path();
	std::string repakExe = settings->GetRepakExePath();

	std::string rpakName = name;
	std::string dllName = rpakName + ".dll";

	// --- Step A: Compile DLL (vswhere: VS2026/2022, any edition) ---
	{
		std::string exportDirStr = exportDir.string();
		// Prefer vswhere; fall back to the common install locations.
		std::string vcvarsCandidates[] = {
			"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe",
			"C:\\Program Files\\Microsoft Visual Studio\\Installer\\vswhere.exe",
		};
		std::string vcvarsPath;
		for (const auto& vswhere : vcvarsCandidates) {
			if (!fs::exists(vswhere))
				continue;
			// Write install path to a temp file via vswhere -property installationPath
			fs::path tmpOut = exportDir / "_vs_install_path.txt";
			std::string q = "cmd /c \"\"" + vswhere + "\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > \"" + tmpOut.string() + "\" 2>nul\"";
			system(q.c_str());
			if (fs::exists(tmpOut)) {
				std::string installPath;
				{
					std::ifstream ifs(tmpOut);
					std::getline(ifs, installPath);
				}
				while (!installPath.empty() && (installPath.back() == '\r' || installPath.back() == '\n' || installPath.back() == ' '))
					installPath.pop_back();
				std::error_code rmEc;
				fs::remove(tmpOut, rmEc);
				if (!installPath.empty()) {
					fs::path cand = fs::path(installPath) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat";
					if (fs::exists(cand)) {
						vcvarsPath = cand.string();
						break;
					}
				}
			}
		}
		if (vcvarsPath.empty()) {
			const char* fallbacks[] = {
				"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
				"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
				"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
			};
			for (const char* fb : fallbacks) {
				if (fs::exists(fb)) {
					vcvarsPath = fb;
					break;
				}
			}
		}
		if (vcvarsPath.empty()) {
			exportPopupMessage += "\n\nBuild FAILED: vcvars64.bat not found (install the VS C++ build tools)";
			return;
		}
		std::string batCmd = "call \"" + vcvarsPath + "\" >nul 2>&1 && cd /d \"" + exportDirStr + "\" && cl /LD /O2 /EHsc /std:c++17 " + name + ".cpp /Fe:" + dllName + " > cl_output.txt 2>&1";
		std::string cmd = "cmd /c \"" + batCmd + "\"";
		system(cmd.c_str());
		if (!fs::exists(exportDir / dllName)) {
			exportPopupMessage += "\n\nBuild FAILED: " + dllName + " not produced (see cl_output.txt)";
			return;
		}
		exportPopupMessage += "\n\nBuild:\n  " + dllName + " OK";
	}
	fs::path jsonMapPath = exportDir / (name + "_repak.json");
	{
		std::string assetsDir = exportDir.string();
		std::replace(assetsDir.begin(), assetsDir.end(), '\\', '/');
		if (!assetsDir.empty() && assetsDir.back() != '/') assetsDir += '/';

		std::string buildDir = assetsDir + "build/";

		auto now = std::chrono::system_clock::now();
		uint32_t epoch = (uint32_t)std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

		std::ofstream jf(jsonMapPath);
		if (jf.good()) {
			jf << "{\n";
			jf << "    \"version\": 8,\n";
			jf << "    \"hasDynamicLibrary\": true,\n";
			// 0x20 is required by the S21 native loader; without it the pak is rejected.
			jf << "    \"headerFlags\": 32,\n";
			jf << "    \"name\": \"" << rpakName << "\",\n";
			jf << "    \"assetsDir\": \"" << assetsDir << "\",\n";
			jf << "    \"outputDir\": \"" << buildDir << "\",\n";
			jf << "    \"buildDate\": " << epoch << ",\n";
			jf << "    \"files\": [\n";
			jf << "        {\n";
			jf << "            \"_type\": \"ui\",\n";
			jf << "            \"_path\": \"ui/" << name << ".rpak\"\n";
			jf << "        }\n";
			jf << "    ]\n";
			jf << "}\n";
			jf.close();
		}
	}

	// --- Step C: Move .ruip into ui/ subfolder (repak expects assetsDir/ui/{name}.ruip) ---
	{
		fs::path uiDir = exportDir / "ui";
		fs::create_directories(uiDir);
		fs::path srcRuip = exportDir / (name + ".ruip");
		fs::path dstRuip = uiDir / (name + ".ruip");
		if (fs::exists(srcRuip)) {
			std::error_code ec;
			fs::copy_file(srcRuip, dstRuip, fs::copy_options::overwrite_existing, ec);
		}
	}

	// --- Step D: Run repak.exe ---
	if (repakExe.empty() || !fs::exists(repakExe)) {
		exportPopupMessage += "\n  repak.exe not configured (set in Settings)";
		return;
	}
	{
		fs::create_directories(exportDir / "build");
		std::string cmd = "cmd /c \"\"" + repakExe + "\" \"" + jsonMapPath.string() + "\"\"";
		printf("[SERE] Running: %s\n", cmd.c_str());
		system(cmd.c_str());
		fs::path rpakOut = exportDir / "build" / (rpakName + ".rpak");
		if (!fs::exists(rpakOut)) {
			exportPopupMessage += "\n  repak FAILED: " + rpakName + ".rpak not produced";
			return;
		}
		exportPopupMessage += "\n  " + rpakName + ".rpak OK";
	}

	// --- Step D: Deploy to game paks/Win64 ---
	if (gamePath.empty() || !fs::exists(gamePath)) {
		exportPopupMessage += "\n  Deploy skipped (GamePath not set)";
		return;
	}
	{
		fs::path paksDir = fs::path(gamePath) / "paks" / "Win64";
		if (!fs::exists(paksDir)) {
			exportPopupMessage += "\n  Deploy skipped (paks/Win64 not found)";
			return;
		}
		std::error_code ec;
		fs::copy_file(exportDir / "build" / (rpakName + ".rpak"), paksDir / (rpakName + ".rpak"), fs::copy_options::overwrite_existing, ec);
		fs::copy_file(exportDir / dllName, paksDir / dllName, fs::copy_options::overwrite_existing, ec);
		exportPopupMessage += "\n\n__GREEN__Deployed to /paks/Win64";
	}
}

void NodeEditor::DrawExportPopup() {
	if (showExportPopup) {
		ImGui::OpenPopup("Export Complete");
		showExportPopup = false;
	}
	if (ImGui::BeginPopupModal("Export Complete", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		size_t greenPos = exportPopupMessage.find("__GREEN__");
		if (greenPos != std::string::npos) {
			ImGui::TextUnformatted(exportPopupMessage.c_str(), exportPopupMessage.c_str() + greenPos);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
			ImGui::TextUnformatted(exportPopupMessage.c_str() + greenPos + 9);
			ImGui::PopStyleColor();
		} else {
			ImGui::TextUnformatted(exportPopupMessage.c_str());
		}
		ImGui::Spacing();
		if (ImGui::Button("OK", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void NodeEditor::RightClickPopup(ImFlow::BaseNode* node) {
	if (node) {

		if (ImGui::MenuItem("Delete")) {
			node->destroy();
		}
		return;
	}

	for (const auto& [categoryName, category] : nodeTypes) {
		if (ImGui::BeginMenu(categoryName.c_str())) {
			for (const auto& [nodeName, nodeType] : category) {
				if (ImGui::MenuItem(nodeName.c_str())) {
					nodeType.AddNode(mINF,render,mINF.getStyleManager());
				}
			}

			ImGui::EndMenu();
		}

	}
#ifdef _DEBUG
	if (ImGui::BeginMenu("Debug")) {
		if (ImGui::MenuItem("Spawn All Node Types")) {
			for (auto& [catName, category] : nodeTypes) {
				for (auto& [name, node] : category) {
					node.AddNode(mINF,render,mINF.getStyleManager());
				}
			}

		}
		ImGui::EndMenu();
	}
#endif
}

void NodeEditor::LinkDroppedPopup(ImFlow::Pin* pin) {
	if (!pin)
		return;

	ImFlow::PinType neededPinType = pin->getType() == ImFlow::PinType_Input ?
		ImFlow::PinType_Output : ImFlow::PinType_Input;

	static std::string searchString;

	// force keyboard focus to the InputText when first showing the window
	if (ImGui::GetCurrentWindow()->Appearing)
		ImGui::SetKeyboardFocusHere();
	// if enter is pressed, select the first thing we find
	bool selectFirstOption = ImGui::InputText("Search", &searchString, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

	for (auto& [catName, category] : nodeTypes)
	{
		bool searchHit = caseInsensitiveSearch(catName, searchString);

		bool hasCategoryHeader = false;
		for (auto& [nodeName, nodeType] : category)
		{
			searchHit |= caseInsensitiveSearch(nodeName, searchString);
			for (auto& pinInfo : nodeType.GetPinInfo())
			{
				searchHit |= caseInsensitiveSearch(pinInfo->name, searchString);

				if (pin->getType() == pinInfo->GetPinType())
					continue;
				if (!pinInfo->CanCreateLink(pin->getProto()))
					continue;
				if (searchString.size() && !searchHit)
					continue;

				if (!hasCategoryHeader)
				{
					ImGui::MenuItem(catName.c_str(), nullptr, nullptr, false);
					hasCategoryHeader = true;
				}

				std::string menuName = std::format("{} > {}", nodeName, pinInfo->name);
				if (selectFirstOption || ImGui::MenuItem(menuName.c_str())) {
					std::shared_ptr<ImFlow::BaseNode> node = nodeType.AddNode(mINF, render, mINF.getStyleManager());
					// create the pin
					if (pinInfo->GetPinType() == ImFlow::PinType_Input) {
						node->inPin(pinInfo->name.c_str())->createLink(pin);
					}
					else {
						node->outPin(pinInfo->name.c_str())->createLink(pin);
					}

					// clear the search for next time and exit
					searchString = "";
					ImGui::CloseCurrentPopup(); // todo: return value for this function so the caller can handle this? we may not be a popup i guess
					return;
				}
			}
		}
	}
}


void NodeEditor::SetStyles(ImFlow::StyleManager& styles) {
	// Grid: deep charcoal with subtle lines
	styles.grid.colors.background = IM_COL32(22, 24, 30, 255);
	styles.grid.colors.grid       = IM_COL32(255, 255, 255, 18);
	styles.grid.colors.subGrid    = IM_COL32(255, 255, 255, 6);
	styles.grid.grid_size          = 48.f;
	styles.grid.grid_subdivisions  = 4.f;

	// Helper: create a pin style with consistent sizing
	auto pin = [](ImU32 col, int shape) {
		auto p = std::make_shared<ImFlow::PinStyle>(col, shape, 4.2f, 5.0f, 4.0f, 1.1f);
		p->extra.link_thickness          = 2.4f;
		p->extra.link_dragged_thickness  = 2.0f;
		p->extra.link_hovered_thickness  = 3.2f;
		p->extra.bg_hover_color          = IM_COL32(255, 255, 255, 25);
		p->extra.outline_color           = IM_COL32(130, 170, 255, 180);
		return p;
	};

	// Pin colors: distinct, desaturated palette that reads well on dark nodes
	styles.AddPinStlye(typeid(TransformResult).name(), pin(IM_COL32(220, 105, 105, 255), 1)); // red square
	styles.AddPinStlye(typeid(TransformSize).name(),   pin(IM_COL32(105, 210, 120, 255), 1)); // green square

	styles.AddPinStlye(typeid(IntVariable).name(),     pin(IM_COL32(100, 180, 210, 255), 0)); // steel blue
	styles.AddPinStlye(typeid(BoolVariable).name(),    pin(IM_COL32(160, 120, 210, 255), 0)); // lavender
	styles.AddPinStlye(typeid(FloatVariable).name(),   pin(IM_COL32(230, 210, 100, 255), 0)); // warm yellow
	styles.AddPinStlye(typeid(Float2Variable).name(),  pin(IM_COL32(140, 225, 120, 255), 0)); // lime
	styles.AddPinStlye(typeid(Float3Variable).name(),  pin(IM_COL32(100, 220, 195, 255), 0)); // teal
	styles.AddPinStlye(typeid(ColorVariable).name(),   pin(IM_COL32(110, 170, 240, 255), 0)); // sky blue
	styles.AddPinStlye(typeid(StringVariable).name(),  pin(IM_COL32(175, 140, 240, 255), 0)); // violet
	styles.AddPinStlye(typeid(AssetVariable).name(),   pin(IM_COL32(220, 130, 230, 255), 0)); // pink

	// Helper: create a node style with consistent body/border
	ImColor titleCol(220, 225, 235, 255);
	auto node = [&](ImU32 headerBg) {
		auto n = std::make_shared<ImFlow::NodeStyle>(headerBg, titleCol, 5.0f);
		n->bg                      = IM_COL32(38, 42, 52, 245);
		n->border_color            = IM_COL32(58, 64, 80, 160);
		n->border_selected_color   = IM_COL32(130, 170, 255, 220);
		n->border_thickness        = 1.0f;
		n->border_selected_thickness = 1.8f;
		n->padding                 = ImVec4(12.f, 5.f, 12.f, 3.f);
		return n;
	};

	// Node header colors: rich but not oversaturated
	styles.AddNodeStlye("Math",         node(IM_COL32( 55,  85, 165, 255)));  // blue
	styles.AddNodeStlye("Transform",    node(IM_COL32( 50, 145,  70, 255)));  // green
	styles.AddNodeStlye("Render",       node(IM_COL32(175,  55,  45, 255)));  // red
	styles.AddNodeStlye("Constant",     node(IM_COL32(130,  55, 155, 255)));  // purple
	styles.AddNodeStlye("Argument",     node(IM_COL32( 95,  55, 170, 255)));  // indigo
	styles.AddNodeStlye("Split Merge",  node(IM_COL32( 45, 135, 155, 255)));  // teal
	styles.AddNodeStlye("Global",       node(IM_COL32( 75,  60, 145, 255)));  // deep violet
	styles.AddNodeStlye("Conditionals", node(IM_COL32(180, 110,  40, 255)));  // amber

	auto errorStyle = node(IM_COL32(190, 40, 35, 255));
	errorStyle->bg = IM_COL32(95, 25, 22, 245);
	styles.SetNodeErrorStyle(errorStyle);
}

