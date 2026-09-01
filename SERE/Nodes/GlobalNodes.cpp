#include "GlobalNodes.h"

// ============================================================================
// FLOAT GLOBAL NODE IMPLEMENTATION MACRO
// Creates a node that reads a float field from globals and outputs FloatVariable
// ============================================================================
#define IMPL_FLOAT_GLOBAL_NODE(ClassName, previewField, exportField, drawLabel, pinName) \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style) \
	:RuiBaseNode(name, category, GetPinInfo(), rend, style) { \
	std::string outName = Variable::UniqueName(); \
	getOut<FloatVariable>(pinName)->behaviour([this, outName]() { \
		return FloatVariable(render.globals.previewField, outName); \
	}); \
} \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style, \
	rapidjson::GenericObject<false, rapidjson::Value> obj) : ClassName(rend, style) {} \
void ClassName::draw() { \
	ImGui::Text(drawLabel " %f", render.globals.previewField); \
} \
void ClassName::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, \
	rapidjson::Document::AllocatorType& allocator) { \
	obj.AddMember("Name", name, allocator); \
	obj.AddMember("Category", category, allocator); \
	RuiBaseNode::Serialize(obj, allocator); \
} \
void ClassName::Export(RuiExportPrototype& proto) { \
	auto out = getOut<FloatVariable>(pinName)->val(); \
	ExportElement<std::string> ele; \
	ele.identifier = out.name; \
	ele.callback = [out](RuiExportPrototype& proto) { \
		if (proto.varsInDataStruct.contains(out.name)) \
			proto.codeLines.push_back(std::format("{} = globals->" exportField ";", out.GetFormattedName(proto))); \
		else \
			proto.codeLines.push_back(std::format("float {} = globals->" exportField ";", out.GetFormattedName(proto))); \
	}; \
	proto.codeElements.push_back(ele); \
} \
std::vector<std::shared_ptr<ImFlow::PinProto>> ClassName::GetPinInfo() { \
	std::vector<std::shared_ptr<ImFlow::PinProto>> info; \
	info.push_back(std::make_shared<ImFlow::OutPinProto<FloatVariable>>(pinName)); \
	return info; \
}

// ============================================================================
// INT→FLOAT GLOBAL NODE (reads int, casts to float for node compatibility)
// ============================================================================
#define IMPL_INT_GLOBAL_NODE(ClassName, previewField, exportField, drawLabel, pinName) \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style) \
	:RuiBaseNode(name, category, GetPinInfo(), rend, style) { \
	std::string outName = Variable::UniqueName(); \
	getOut<FloatVariable>(pinName)->behaviour([this, outName]() { \
		return FloatVariable((float)render.globals.previewField, outName); \
	}); \
} \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style, \
	rapidjson::GenericObject<false, rapidjson::Value> obj) : ClassName(rend, style) {} \
void ClassName::draw() { \
	ImGui::Text(drawLabel " %d", render.globals.previewField); \
} \
void ClassName::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, \
	rapidjson::Document::AllocatorType& allocator) { \
	obj.AddMember("Name", name, allocator); \
	obj.AddMember("Category", category, allocator); \
	RuiBaseNode::Serialize(obj, allocator); \
} \
void ClassName::Export(RuiExportPrototype& proto) { \
	auto out = getOut<FloatVariable>(pinName)->val(); \
	ExportElement<std::string> ele; \
	ele.identifier = out.name; \
	ele.callback = [out](RuiExportPrototype& proto) { \
		if (proto.varsInDataStruct.contains(out.name)) \
			proto.codeLines.push_back(std::format("{} = (float)globals->" exportField ";", out.GetFormattedName(proto))); \
		else \
			proto.codeLines.push_back(std::format("float {} = (float)globals->" exportField ";", out.GetFormattedName(proto))); \
	}; \
	proto.codeElements.push_back(ele); \
} \
std::vector<std::shared_ptr<ImFlow::PinProto>> ClassName::GetPinInfo() { \
	std::vector<std::shared_ptr<ImFlow::PinProto>> info; \
	info.push_back(std::make_shared<ImFlow::OutPinProto<FloatVariable>>(pinName)); \
	return info; \
}

// ============================================================================
// VECTOR3 GLOBAL NODE
// ============================================================================
#define IMPL_VEC3_GLOBAL_NODE(ClassName, previewField, exportField, drawLabel, pinName) \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style) \
	:RuiBaseNode(name, category, GetPinInfo(), rend, style) { \
	std::string outName = Variable::UniqueName(); \
	minVal = -10000; maxVal = 10000; \
	getOut<Float3Variable>(pinName)->behaviour([this, outName]() { \
		return Float3Variable(render.globals.previewField[0], \
			render.globals.previewField[1], render.globals.previewField[2], outName); \
	}); \
} \
ClassName::ClassName(RenderInstance& rend, ImFlow::StyleManager& style, \
	rapidjson::GenericObject<false, rapidjson::Value> obj) : ClassName(rend, style) {} \
void ClassName::draw() { \
	ImGui::PushItemWidth(240); \
	ImGui::SliderFloat3(drawLabel, render.globals.previewField, minVal, maxVal); \
	ImGui::PopItemWidth(); \
} \
void ClassName::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, \
	rapidjson::Document::AllocatorType& allocator) { \
	obj.AddMember("Name", name, allocator); \
	obj.AddMember("Category", category, allocator); \
	RuiBaseNode::Serialize(obj, allocator); \
} \
void ClassName::Export(RuiExportPrototype& proto) { \
	auto out = getOut<Float3Variable>(pinName)->val(); \
	ExportElement<std::string> ele; \
	ele.identifier = out.name; \
	ele.callback = [out](RuiExportPrototype& proto) { \
		if (proto.varsInDataStruct.contains(out.name)) \
			proto.codeLines.push_back(std::format("{} = globals->" exportField ";", out.GetFormattedName(proto))); \
		else \
			proto.codeLines.push_back(std::format("Vector3 {} = globals->" exportField ";", out.GetFormattedName(proto))); \
	}; \
	proto.codeElements.push_back(ele); \
} \
std::vector<std::shared_ptr<ImFlow::PinProto>> ClassName::GetPinInfo() { \
	std::vector<std::shared_ptr<ImFlow::PinProto>> info; \
	info.push_back(std::make_shared<ImFlow::OutPinProto<Float3Variable>>(pinName)); \
	return info; \
}

// ============================================================================
// ALL NODE IMPLEMENTATIONS
// ============================================================================

// --- Time ---
IMPL_FLOAT_GLOBAL_NODE(TimeNode, currentTime, "currentTime", "Time", "Time")
IMPL_FLOAT_GLOBAL_NODE(UiTimeNode, uiTime, "uiTime", "UI Time", "Time")

// --- Viewport ---
IMPL_FLOAT_GLOBAL_NODE(ScreenWidthNode, screenWidth, "screenWidth", "Width", "ScreenWidth")
IMPL_FLOAT_GLOBAL_NODE(ScreenHeightNode, screenHeight, "screenHeight", "Height", "ScreenHeight")

// --- Camera ---
IMPL_VEC3_GLOBAL_NODE(LocalPlayerPosNode, localPlayerPos, "localPlayerPos", "Player Pos", "LocalPlayerPos")
IMPL_VEC3_GLOBAL_NODE(CamOrgLocalNode, localPlayerPos, "camOrgLocal", "Cam Local", "CamLocal")

// --- Player State (float) ---
IMPL_FLOAT_GLOBAL_NODE(ADSFracNode, adsFracValue, "globalAdsFrac", "ADS", "AdsFrac")
IMPL_FLOAT_GLOBAL_NODE(CrosshairADSFracNode, crosshairADSFrac, "playerCrosshairADSFrac", "Crosshair ADS", "Value")

// --- Player State (int) ---
IMPL_INT_GLOBAL_NODE(IsAliveNode, isAlive, "playerIsAlive", "Alive", "Value")
IMPL_INT_GLOBAL_NODE(IsSpectatorNode, isSpectator, "playerIsSpectator", "Spectator", "Value")
IMPL_INT_GLOBAL_NODE(IsViewingDeathScreenNode, isViewingDeathScreen, "playerIsViewingDeathScreen", "Death Screen", "Value")
IMPL_INT_GLOBAL_NODE(IsThirdPersonNode, isThirdPerson, "playerIsThirdPerson", "3rd Person", "Value")
IMPL_INT_GLOBAL_NODE(IsPhaseShiftedNode, isPhaseShifted, "playerIsPhaseShifted", "Phase Shift", "Value")
IMPL_INT_GLOBAL_NODE(IsUsingControllerNode, isUsingController, "playerIsUsingController", "Controller", "Value")
IMPL_INT_GLOBAL_NODE(HasOpenMenuNode, hasOpenMenu, "playerHasOpenMenu", "Menu Open", "Value")
IMPL_INT_GLOBAL_NODE(IsOneHandedNode, isOneHanded, "playerIsOneHanded", "One Handed", "Value")
IMPL_INT_GLOBAL_NODE(SniperScopeEquippedNode, sniperScopeEquipped, "playerSniperScopeEquipped", "Sniper Scope", "Value")
IMPL_INT_GLOBAL_NODE(IsDrivingHoverVehicleNode, isDrivingHoverVehicle, "playerIsDrivingHoverVehicle", "Hover Vehicle", "Value")
IMPL_INT_GLOBAL_NODE(IsPureSpectatorNode, isPureSpectator, "playerIsPureSpectator", "Pure Spectator", "Value")

// --- Kill Replay ---
IMPL_INT_GLOBAL_NODE(KillReplayIsWatchingNode, killReplayIsWatching, "killReplayIsWatching", "Kill Replay", "Value")
IMPL_FLOAT_GLOBAL_NODE(KillReplayChangeTimeNode, killReplayChangeTime, "killReplayChangeTime", "Replay Time", "Value")

// --- Game State ---
IMPL_INT_GLOBAL_NODE(AnnouncementIsActiveNode, announcementIsActive, "gameAnnouncementIsActive", "Announcement", "Value")
IMPL_FLOAT_GLOBAL_NODE(AnnouncementChangeTimeNode, announcementChangeTime, "gameAnnouncementChangeTime", "Ann. Time", "Value")
IMPL_VEC3_GLOBAL_NODE(FriendlyTeamColorNode, friendlyTeamColor, "gameFriendlyTeamColor", "Friendly Color", "Value")
IMPL_VEC3_GLOBAL_NODE(EnemyTeamColorNode, enemyTeamColor, "gameEnemyTeamColor", "Enemy Color", "Value")
IMPL_VEC3_GLOBAL_NODE(PartyTeamColorNode, partyTeamColor, "gamePartyTeamColor", "Party Color", "Value")

// --- Misc ---
IMPL_INT_GLOBAL_NODE(NxModeNode, nxMode, "nxMode", "NX Mode", "Value")

// ============================================================================
// REGISTER ALL NODES
// ============================================================================
void AddGlobalNodes(NodeEditor& editor) {
	// Time
	editor.AddNodeType<TimeNode>();
	editor.AddNodeType<UiTimeNode>();

	// Viewport
	editor.AddNodeType<ScreenWidthNode>();
	editor.AddNodeType<ScreenHeightNode>();

	// Camera
	editor.AddNodeType<LocalPlayerPosNode>();
	editor.AddNodeType<CamOrgLocalNode>();

	// Player State (float)
	editor.AddNodeType<ADSFracNode>();
	editor.AddNodeType<CrosshairADSFracNode>();

	// Player State (bool/int)
	editor.AddNodeType<IsAliveNode>();
	editor.AddNodeType<IsSpectatorNode>();
	editor.AddNodeType<IsViewingDeathScreenNode>();
	editor.AddNodeType<IsThirdPersonNode>();
	editor.AddNodeType<IsPhaseShiftedNode>();
	editor.AddNodeType<IsUsingControllerNode>();
	editor.AddNodeType<HasOpenMenuNode>();
	editor.AddNodeType<IsOneHandedNode>();
	editor.AddNodeType<SniperScopeEquippedNode>();
	editor.AddNodeType<IsDrivingHoverVehicleNode>();
	editor.AddNodeType<IsPureSpectatorNode>();

	// Kill Replay
	editor.AddNodeType<KillReplayIsWatchingNode>();
	editor.AddNodeType<KillReplayChangeTimeNode>();

	// Game State
	editor.AddNodeType<AnnouncementIsActiveNode>();
	editor.AddNodeType<AnnouncementChangeTimeNode>();
	editor.AddNodeType<FriendlyTeamColorNode>();
	editor.AddNodeType<EnemyTeamColorNode>();
	editor.AddNodeType<PartyTeamColorNode>();

	// NxMode omitted — not relevant for Apex
}
