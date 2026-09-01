#pragma once

#include "RuiNodeEditor/RuiNodeEditor.h"
#include "RuiRendering/RenderFunctions.h"

void AddRenderNodes(NodeEditor& editor);

class AssetRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Render Image Image Mask";
	static inline std::string category = "Image Render";

	explicit AssetRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit AssetRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();

	bool maskFlag;
	int layer;
};

class AssetCircleRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Render Image Circle Mask";
	static inline std::string category = "Image Render";

	explicit AssetCircleRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit AssetCircleRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();

	int layer;
};


class TextStyleNode : public RuiBaseNode {
public:
	static inline std::string name = "Text Style";
	static inline std::string category = "Text Render";

	explicit TextStyleNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit TextStyleNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();

private:

	Font_t* currentFont;
};

class TextSizeNode : public RuiBaseNode {
public:
	static inline std::string name = "Text Size";
	static inline std::string category = "Text Render";

	explicit TextSizeNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit TextSizeNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
};

class TextRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Text Render";
	static inline std::string category = "Text Render";

	explicit TextRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit TextRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
	int layer;
};

class VideoRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Render Video";
	static inline std::string category = "Image Render";

	explicit VideoRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit VideoRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
	int layer;
	int opts;
};

class CameraRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Render Camera";
	static inline std::string category = "Image Render";

	explicit CameraRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit CameraRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
	int layer;
	int opts;
};

class NestedRenderNode : public RuiBaseNode {
public:
	static inline std::string name = "Render Nested";
	static inline std::string category = "Image Render";

	explicit NestedRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit NestedRenderNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
	int layer;
	int opts;
};

