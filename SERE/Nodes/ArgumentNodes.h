#pragma once

#include "RuiNodeEditor/RuiNodeEditor.h"



void AddArgumentNodes(NodeEditor& editor);

class IntArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Integer Arg";
	static inline std::string category = "Argument";

	explicit IntArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit IntArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class BoolArgNode : public RuiBaseNode
{	
public:
	static inline std::string name = "Boolean Arg";
	static inline std::string category = "Argument";

	explicit BoolArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit BoolArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class FloatArgNode : public RuiBaseNode
{	
public:
	static inline std::string name = "Float Arg";
	static inline std::string category = "Argument";

	explicit FloatArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit FloatArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class GametimeArgNode : public RuiBaseNode
{	
public:
	static inline std::string name = "Gametime Arg";
	static inline std::string category = "Argument";

	explicit GametimeArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit GametimeArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class Float2ArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Vector2 Arg";
	static inline std::string category = "Argument";

	explicit Float2ArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit Float2ArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class Float3ArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Vector3 Arg";
	static inline std::string category = "Argument";

	explicit Float3ArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit Float3ArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class ColorArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Color Arg";
	static inline std::string category = "Argument";

	explicit ColorArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit ColorArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class StringArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "String Arg";
	static inline std::string category = "Argument";

	explicit StringArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit StringArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class AssetArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Asset Arg";
	static inline std::string category = "Argument";

	explicit AssetArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit AssetArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class UiHandleArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Ui Handle Arg";
	static inline std::string category = "Argument";

	explicit UiHandleArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit UiHandleArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class WalltimeArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Walltime Arg";
	static inline std::string category = "Argument";

	explicit WalltimeArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit WalltimeArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class ImageArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Image Arg";
	static inline std::string category = "Argument";

	explicit ImageArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit ImageArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class FontFaceArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Font Face Arg";
	static inline std::string category = "Argument";

	explicit FontFaceArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit FontFaceArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class FontHashArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Font Hash Arg";
	static inline std::string category = "Argument";

	explicit FontHashArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit FontHashArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class ArrayArgNode : public RuiBaseNode
{
public:
	static inline std::string name = "Array Arg";
	static inline std::string category = "Argument";

	explicit ArrayArgNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit ArrayArgNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
private:
	std::string argName;
};

class LocalizeNode : public RuiBaseNode
{
public:
	static inline std::string name = "Localize";
	static inline std::string category = "Code";

	explicit LocalizeNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit LocalizeNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
};

class SNPrintFNode : public RuiBaseNode
{
public:
	static inline std::string name = "SNPrintF";
	static inline std::string category = "Code";

	explicit SNPrintFNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit SNPrintFNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
};

class SetHiddenNode : public RuiBaseNode
{
public:
	static inline std::string name = "Set Hidden";
	static inline std::string category = "Code";

	explicit SetHiddenNode(RenderInstance& prot,ImFlow::StyleManager& styles);
	explicit SetHiddenNode(RenderInstance& prot,ImFlow::StyleManager& styles, rapidjson::GenericObject<false,rapidjson::Value> obj);
	void draw() override;
	void Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj,rapidjson::Document::AllocatorType& allocator) override;
	void Export(RuiExportPrototype& proto) override;

	static std::vector<std::shared_ptr<ImFlow::PinProto>> GetPinInfo();
};

