#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <functional>
#include <filesystem>

#include "imgui/ImNodeFlow.h"
#include "RuiRendering/RenderManager.h"
#define RAPIDJSON_HAS_STDSTRING 1
#include "ThirdParty/rapidjson/document.h"

#include "RuiNodeEditor/RuiBaseNode.h"
#include "RuiNodeEditor/RuiVariables.h"
#include "RuiNodeEditor/RuiExportPrototype.h"
#include "Settings.h"

namespace fs = std::filesystem;



class NodeEditor{
private:
	ImFlow::ImNodeFlow mINF;
	RenderInstance& render;
	Settings* settings = nullptr;

	std::map<std::string,NodeCategory> nodeTypes;
	bool showExportPopup = false;
	std::string exportPopupMessage;

	std::string m_currentFilePath;
	bool m_dirty = false;
	bool m_showUnsavedPrompt = false;
	std::function<void()> m_pendingAction;

	void BuildAndDeploy(const fs::path& exportDir, const std::string& name);
	void SerializeToPath(const fs::path& path);
	void DeserializeFromPath(const fs::path& path);
	void DeserializeDocument(rapidjson::Document& doc);
public:
	NodeEditor(RenderInstance& rend);
	void SetSettings(Settings* s) { settings = s; }
	void MarkDirty() { m_dirty = true; }
	bool IsDirty() const { return m_dirty; }
	std::string GetWindowTitle() const;
	void SetStyles(ImFlow::StyleManager& styles);
	void RightClickPopup(ImFlow::BaseNode* node);
	void LinkDroppedPopup(ImFlow::Pin* pin);
	void Draw();
	void New();
	void Save();
	void SaveAs();
	void Load();
	void Export();
	void DrawExportPopup();
	void DrawUnsavedPrompt();
	void Clear();

	// Control-channel surface (see Bridge/ControlServer). All of these run on
	// the main thread; nothing here may be called from the socket thread.
	std::string SerializeToString();
	bool DeserializeFromString(const std::string& json, std::string& error);
	void LoadFromPath(const fs::path& path);
	void SaveToPath(const fs::path& path);
	std::string ExportToPath(const fs::path& path);
	std::string CurrentFilePath() const { return m_currentFilePath; }
	size_t NodeCount() { return mINF.getNodesCount(); }

	template<class T> void AddNodeType() {
		
		
		const std::string& category = T::category;
		const std::string& name = T::name;
		NodeType type = CreateNodeType<T>();
		if (nodeTypes.contains(category)) {
			nodeTypes[category].emplace(name,type);
			return;
		}
		std::map<std::string,NodeType> newCategory;
		newCategory.emplace(name,type);
		nodeTypes.emplace(category,newCategory);
		
	}
};



	
