#pragma once

#include "RuiNodeEditor/RuiNodeEditor.h"

// Macro for simple float global nodes
#define DECLARE_FLOAT_GLOBAL_NODE(ClassName, NodeName) \
class ClassName : public RuiBaseNode { \
public: \
	static inline std::string name = NodeName; \
	static inline std::string category = "Globals"; \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles); \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj); \
	void draw() override; \
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) override; \
	void Export(RuiExportPrototype& proto) override; \
	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo(); \
};

// Macro for simple int global nodes
#define DECLARE_INT_GLOBAL_NODE(ClassName, NodeName) \
class ClassName : public RuiBaseNode { \
public: \
	static inline std::string name = NodeName; \
	static inline std::string category = "Globals"; \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles); \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj); \
	void draw() override; \
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) override; \
	void Export(RuiExportPrototype& proto) override; \
	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo(); \
};

// Macro for Vector3 global nodes
#define DECLARE_VEC3_GLOBAL_NODE(ClassName, NodeName) \
class ClassName : public RuiBaseNode { \
public: \
	static inline std::string name = NodeName; \
	static inline std::string category = "Globals"; \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles); \
	explicit ClassName(RenderInstance& prot, ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj); \
	void draw() override; \
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) override; \
	void Export(RuiExportPrototype& proto) override; \
	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo(); \
private: \
	float minVal; \
	float maxVal; \
};

// --- Time ---
DECLARE_FLOAT_GLOBAL_NODE(TimeNode, "Current Time")
DECLARE_FLOAT_GLOBAL_NODE(UiTimeNode, "UI Time")

// --- Viewport ---
DECLARE_FLOAT_GLOBAL_NODE(ScreenWidthNode, "Screen Width")
DECLARE_FLOAT_GLOBAL_NODE(ScreenHeightNode, "Screen Height")

// --- Camera ---
DECLARE_VEC3_GLOBAL_NODE(LocalPlayerPosNode, "Player Position")
DECLARE_VEC3_GLOBAL_NODE(CamOrgLocalNode, "Camera Local Pos")

// --- Player State (float) ---
DECLARE_FLOAT_GLOBAL_NODE(ADSFracNode, "ADS Fraction")
DECLARE_FLOAT_GLOBAL_NODE(CrosshairADSFracNode, "Crosshair ADS Frac")

// --- Player State (int → float for node compat) ---
DECLARE_FLOAT_GLOBAL_NODE(IsAliveNode, "Player Is Alive")
DECLARE_FLOAT_GLOBAL_NODE(IsSpectatorNode, "Player Is Spectator")
DECLARE_FLOAT_GLOBAL_NODE(IsViewingDeathScreenNode, "Viewing Death Screen")
DECLARE_FLOAT_GLOBAL_NODE(IsThirdPersonNode, "Is Third Person")
DECLARE_FLOAT_GLOBAL_NODE(IsPhaseShiftedNode, "Is Phase Shifted")
DECLARE_FLOAT_GLOBAL_NODE(IsUsingControllerNode, "Is Using Controller")
DECLARE_FLOAT_GLOBAL_NODE(HasOpenMenuNode, "Has Open Menu")
DECLARE_FLOAT_GLOBAL_NODE(IsOneHandedNode, "Is One Handed")
DECLARE_FLOAT_GLOBAL_NODE(SniperScopeEquippedNode, "Sniper Scope Equipped")
DECLARE_FLOAT_GLOBAL_NODE(IsDrivingHoverVehicleNode, "Driving Hover Vehicle")
DECLARE_FLOAT_GLOBAL_NODE(IsPureSpectatorNode, "Is Pure Spectator")

// --- Kill Replay ---
DECLARE_FLOAT_GLOBAL_NODE(KillReplayIsWatchingNode, "Kill Replay Active")
DECLARE_FLOAT_GLOBAL_NODE(KillReplayChangeTimeNode, "Kill Replay Change Time")

// --- Game State ---
DECLARE_FLOAT_GLOBAL_NODE(AnnouncementIsActiveNode, "Announcement Active")
DECLARE_FLOAT_GLOBAL_NODE(AnnouncementChangeTimeNode, "Announcement Change Time")
DECLARE_VEC3_GLOBAL_NODE(FriendlyTeamColorNode, "Friendly Team Color")
DECLARE_VEC3_GLOBAL_NODE(EnemyTeamColorNode, "Enemy Team Color")
DECLARE_VEC3_GLOBAL_NODE(PartyTeamColorNode, "Party Team Color")

// --- Misc ---
DECLARE_FLOAT_GLOBAL_NODE(NxModeNode, "NX Mode")

void AddGlobalNodes(NodeEditor& editor);
