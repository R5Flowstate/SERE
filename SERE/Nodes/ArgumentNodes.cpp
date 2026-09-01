#include "ArgumentNodes.h"
#include "imgui/imgui_stdlib.h"
#include "Imgui/ImNodeFlow.h"
#include "CustomImGuiWidgets.h"



IntArgNode::IntArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<IntVariable>("Value")->behaviour([this]() {
		int val = 0;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(int)))
			val = std::any_cast<int>(render.arguments[argName]);
		return IntVariable(val,argName);

	});
}

IntArgNode::IntArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):IntArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}


void IntArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	int val;
	if(any.type()==typeid(int))
		val = std::any_cast<int>(any);
	else
		val = 0;
	ImGui::InputInt("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void IntArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void IntArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::INT);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> IntArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<IntVariable>>("Value"));
	return info;
}

BoolArgNode::BoolArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<BoolVariable>("Value")->behaviour([this]() {
		int val = 0;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(int)))
			val = std::any_cast<int>(render.arguments[argName]);
		return BoolVariable(val,argName);
	});
}

BoolArgNode::BoolArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):BoolArgNode(rend,style) {
	
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}


void BoolArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	int val;
	if(any.type()==typeid(int))
		val = std::any_cast<int>(any);
	else
		val = 0;
	bool bval = val;
	ImGui::Checkbox("Default Value",&bval);
	val = bval;
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void BoolArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void BoolArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::BOOL);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> BoolArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<BoolVariable>>("Value"));
	return info;
}

FloatArgNode::FloatArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<FloatVariable>("Value")->behaviour([this]() {
		float val = 0.f;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(float)))
			val = std::any_cast<float>(render.arguments[argName]);
		return FloatVariable(val,argName);
	});
}

FloatArgNode::FloatArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):FloatArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void FloatArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	float val;
	if(any.type()==typeid(float))
		val = std::any_cast<float>(any);
	else
		val = 0.f;
	ImGui::InputFloat("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void FloatArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void FloatArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::FLOAT);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> FloatArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<FloatVariable>>("Value"));
	return info;
}

GametimeArgNode::GametimeArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<FloatVariable>("Value")->behaviour([this]() {
		float val = 0.f;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(float)))
			val = std::any_cast<float>(render.arguments[argName]);
		return FloatVariable(val,argName);
	});
}

GametimeArgNode::GametimeArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):GametimeArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void GametimeArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	float val;
	if(any.type()==typeid(float))
		val = std::any_cast<float>(any);
	else
		val = 0.f;
	ImGui::InputFloat("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void GametimeArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void GametimeArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::GAMETIME);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> GametimeArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<FloatVariable>>("Value"));
	return info;
}

Float2ArgNode::Float2ArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<Float2Variable>("Value")->behaviour([this]() {
		Vector2 val(0.f,0.f);
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(Vector2)))
			val = std::any_cast<Vector2>(render.arguments[argName]);
		return Float2Variable(
			val,
			argName);

	});
}

Float2ArgNode::Float2ArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):Float2ArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void Float2ArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	Vector2 val = Vector2(0.f,0.f);
	if(any.type()==typeid(Vector2))
		val = std::any_cast<Vector2>(any);
	ImGui::InputFloat2("Default Value",&val.x);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void Float2ArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void Float2ArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::FLOAT2);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> Float2ArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<Float2Variable>>("Value"));
	return info;
}

Float3ArgNode::Float3ArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<Float3Variable>("Value")->behaviour([this]() {
		Vector3 val(0.f,0.f,0.f);
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(Vector3)))
			val = std::any_cast<Vector3>(render.arguments[argName]);
		return Float3Variable(
			val,
			argName);

	});
}

Float3ArgNode::Float3ArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):Float3ArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void Float3ArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	Vector3 val = Vector3(0.f,0.f,0.f);
	if(any.type()==typeid(Vector3))
		val = std::any_cast<Vector3>(any);
	ImGui::InputFloat3("Default Value",&val.x);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void Float3ArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void Float3ArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::FLOAT3);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> Float3ArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<Float3Variable>>("Value"));
	return info;
}

ColorArgNode::ColorArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<ColorVariable>("Value")->behaviour([this]() {
		Color val(1.f,1.f,1.f,1.f);
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(int)))
			val = std::any_cast<Color>(render.arguments[argName]);
		return ColorVariable(
			val,
			argName);
	});
}

ColorArgNode::ColorArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):ColorArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void ColorArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	Color val = Color(0.f,0.f,0.f,1.f);
	if(any.type()==typeid(Color))
		val = std::any_cast<Color>(any);
	ImGui::ColorPicker4("Default Value",&val.red);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void ColorArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void ColorArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::COLOR_ALPHA);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> ColorArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<ColorVariable>>("Value"));
	return info;
}

StringArgNode::StringArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<StringVariable>("Value")->behaviour([this]() {
		std::string val = "";
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(std::string)))
			val = std::any_cast<std::string>(render.arguments[argName]);
		return StringVariable(val,argName);

	});
}

StringArgNode::StringArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):StringArgNode(rend,style) {

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void StringArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	std::string val = "";
	if(any.type()==typeid(std::string))
		val = std::any_cast<std::string>(any);
	ImGui::InputText("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void StringArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void StringArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::STRING);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> StringArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<StringVariable>>("Value"));
	return info;
}

AssetArgNode::AssetArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<AssetVariable>("Value")->behaviour([this]() {
		std::string val = "white";
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(std::string)))
			val = std::any_cast<std::string>(render.arguments[argName]);
		return AssetVariable(val,Variable::UniqueName());
	});
}

void AssetArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::ASSET);
	auto& out = getOut<AssetVariable>("Value")->val();
	proto.AddDataVariable(out);
	ExportElement<std::string> ele;
#if _DEBUG
	ele.sourceNodeName = typeid(*this).name();
#endif
	ele.dependencys = { argName };
	ele.identifier = out.name;
	ele.callback =[this](RuiExportPrototype& proto) {
		auto& out = getOut<AssetVariable>("Value")->val();
		proto.codeLines.push_back(std::format("{} = funcs->LoadAsset(inst, {}, data->{}, 0ull);",out.GetFormattedName(proto),out.GetFormattedName(proto),argName));
	};
	proto.codeElements.push_back(ele);
}

AssetArgNode::AssetArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):AssetArgNode(rend,style){

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void AssetArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::string val = "white";
	if(render.arguments.contains(argName)&&(render.arguments[argName].type() == typeid(std::string)))
		val = std::any_cast<std::string>(render.arguments[argName]);
	uint32_t hash = loadAsset(val.c_str());
	if (AtlasImageButton("AssetButton", imageAssetMap[hash])) {
		ImGui::OpenPopup("Asset Selection");
	}
	AssetSelectionPopup("Default Value",&hash);
	val = imageAssetMap[hash].name;
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void AssetArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> AssetArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<AssetVariable>>("Value"));
	return info;
}


UiHandleArgNode::UiHandleArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	getOut<IntVariable>("Value")->behaviour([this]() {
		int val = 0;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(int)))
			val = std::any_cast<int>(render.arguments[argName]);
		return IntVariable(val,argName);
	});
}

void UiHandleArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::UIHANDLE);
}

UiHandleArgNode::UiHandleArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):UiHandleArgNode(rend,style){

	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();

}

void UiHandleArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	int val = 0;
	if(any.type()==typeid(int))
		val = std::any_cast<int>(any);
	ImGui::InputInt("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void UiHandleArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> UiHandleArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<IntVariable>>("Value"));
	return info;
}

WalltimeArgNode::WalltimeArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	getOut<WallTimeVariable>("Value")->behaviour([this]() {
		uint64_t val = 0;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(uint64_t)))
			val = std::any_cast<uint64_t>(render.arguments[argName]);
		return WallTimeVariable(val,argName);
	});
}

WalltimeArgNode::WalltimeArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):WalltimeArgNode(rend,style) {
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void WalltimeArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	uint64_t val = 0;
	if(any.type()==typeid(uint64_t))
		val = std::any_cast<uint64_t>(any);
	ImGui::InputScalar("Default Value", ImGuiDataType_U64, &val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void WalltimeArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void WalltimeArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::WALLTIME);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> WalltimeArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<WallTimeVariable>>("Value"));
	return info;
}

ImageArgNode::ImageArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	getOut<AssetVariable>("Value")->behaviour([this]() {
		std::string val = "white";
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(std::string)))
			val = std::any_cast<std::string>(render.arguments[argName]);
		return AssetVariable(val,Variable::UniqueName());
	});
}

void ImageArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::IMAGE);
	auto& out = getOut<AssetVariable>("Value")->val();
	proto.AddDataVariable(out);
	ExportElement<std::string> ele;
#if _DEBUG
	ele.sourceNodeName = typeid(*this).name();
#endif
	ele.dependencys = { argName };
	ele.identifier = out.name;
	ele.callback =[this](RuiExportPrototype& proto) {
		auto& out = getOut<AssetVariable>("Value")->val();
		proto.codeLines.push_back(std::format("{} = funcs->LoadAsset(inst, {}, data->{}, 0ull);",out.GetFormattedName(proto),out.GetFormattedName(proto),argName));
	};
	proto.codeElements.push_back(ele);
}

ImageArgNode::ImageArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):ImageArgNode(rend,style){
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void ImageArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::string val = "white";
	if(render.arguments.contains(argName)&&(render.arguments[argName].type() == typeid(std::string)))
		val = std::any_cast<std::string>(render.arguments[argName]);
	uint32_t hash = loadAsset(val.c_str());
	if (AtlasImageButton("AssetButton", imageAssetMap[hash])) {
		ImGui::OpenPopup("Asset Selection");
	}
	AssetSelectionPopup("Default Value",&hash);
	val = imageAssetMap[hash].name;
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void ImageArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> ImageArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<AssetVariable>>("Value"));
	return info;
}

FontFaceArgNode::FontFaceArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	getOut<StringVariable>("Value")->behaviour([this]() {
		std::string val = "";
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(std::string)))
			val = std::any_cast<std::string>(render.arguments[argName]);
		return StringVariable(val,argName);
	});
}

FontFaceArgNode::FontFaceArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):FontFaceArgNode(rend,style) {
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void FontFaceArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	std::string val = "";
	if(any.type()==typeid(std::string))
		val = std::any_cast<std::string>(any);
	ImGui::InputText("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void FontFaceArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void FontFaceArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::FONT_FACE);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> FontFaceArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<StringVariable>>("Value"));
	return info;
}

FontHashArgNode::FontHashArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	getOut<IntVariable>("Value")->behaviour([this]() {
		int val = 0;
		if(render.arguments.contains(argName) && (render.arguments[argName].type()==typeid(int)))
			val = std::any_cast<int>(render.arguments[argName]);
		return IntVariable(val,argName);
	});
}

FontHashArgNode::FontHashArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):FontHashArgNode(rend,style) {
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void FontHashArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	std::any& any = render.arguments[argName];
	int val = 0;
	if(any.type()==typeid(int))
		val = std::any_cast<int>(any);
	ImGui::InputInt("Default Value",&val);
	render.arguments[argName] = val;
	ImGui::PopItemWidth();
}

void FontHashArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void FontHashArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::FONT_HASH);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> FontHashArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<IntVariable>>("Value"));
	return info;
}

ArrayArgNode::ArrayArgNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	getOut<IntVariable>("Value")->behaviour([this]() {
		return IntVariable(0,argName);
	});
}

ArrayArgNode::ArrayArgNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):ArrayArgNode(rend,style) {
	if(obj.HasMember("ArgName")&&obj["ArgName"].IsString())
		argName = obj["ArgName"].GetString();
}

void ArrayArgNode::draw() {
	ImGui::PushItemWidth(90);
	ImGui::InputText("Name",&argName);
	if(!render.arguments.contains(argName) || render.arguments[argName].type()!=typeid(uint64_t))
		render.arguments[argName] = (uint64_t)0;
	ImGui::Text("array slot (8 bytes)");
	ImGui::PopItemWidth();
}

void ArrayArgNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("ArgName",argName,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void ArrayArgNode::Export(RuiExportPrototype& proto) {
	proto.arguments.emplace(argName,VariableType::ARRAY);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> ArrayArgNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::OutPinProto<IntVariable>>("Value"));
	return info;
}

LocalizeNode::LocalizeNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	std::string outName = Variable::UniqueName();
	getOut<StringVariable>("Value")->behaviour([this, outName]() {
		StringVariable key = getInVal<StringVariable>("Key");
		return StringVariable(key.value, outName);
	});
}

LocalizeNode::LocalizeNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):LocalizeNode(rend,style) {}

void LocalizeNode::draw() {}

void LocalizeNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void LocalizeNode::Export(RuiExportPrototype& proto) {
	auto out = getOut<StringVariable>("Value")->val();
	auto key = getInVal<StringVariable>("Key");
	proto.AddDataVariable(key);
	proto.AddDataVariable(out);
	ExportElement<std::string> ele;
#if _DEBUG
	ele.sourceNodeName = typeid(*this).name();
#endif
	ele.identifier = out.name;
	ele.dependencys = { key.name };
	ele.callback = [out, key](RuiExportPrototype& proto) {
		if (proto.varsInDataStruct.contains(out.name) || proto.arguments.contains(out.name))
			proto.codeLines.push_back(std::format("{} = funcs->Localize(inst, {}, 0, 0, 0, 0, 0);", out.GetFormattedName(proto), key.GetFormattedName(proto)));
		else
			proto.codeLines.push_back(std::format("const char* {} = funcs->Localize(inst, {}, 0, 0, 0, 0, 0);", out.GetFormattedName(proto), key.GetFormattedName(proto)));
	};
	proto.codeElements.push_back(ele);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> LocalizeNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<StringVariable>>("Key", ImFlow::ConnectionFilter::SameType(), StringVariable("")));
	info.push_back(std::make_shared<ImFlow::OutPinProto<StringVariable>>("Value"));
	return info;
}

SNPrintFNode::SNPrintFNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	std::string outName = Variable::UniqueName();
	getOut<StringVariable>("Value")->behaviour([this, outName]() {
		StringVariable fmt = getInVal<StringVariable>("Format");
		StringVariable arg = getInVal<StringVariable>("Arg");
		return StringVariable(fmt.value, outName);
	});
}

SNPrintFNode::SNPrintFNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):SNPrintFNode(rend,style) {}

void SNPrintFNode::draw() {}

void SNPrintFNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void SNPrintFNode::Export(RuiExportPrototype& proto) {
	auto out = getOut<StringVariable>("Value")->val();
	auto fmt = getInVal<StringVariable>("Format");
	auto arg = getInVal<StringVariable>("Arg");
	proto.AddDataVariable(fmt);
	proto.AddDataVariable(arg);
	proto.AddDataVariable(out);
	ExportElement<std::string> ele;
#if _DEBUG
	ele.sourceNodeName = typeid(*this).name();
#endif
	ele.identifier = out.name;
	ele.dependencys = { fmt.name, arg.name };
	ele.callback = [out, fmt, arg](RuiExportPrototype& proto) {
		if (proto.varsInDataStruct.contains(out.name) || proto.arguments.contains(out.name))
			proto.codeLines.push_back(std::format("{} = funcs->SNPrintF(inst, {}, {});", out.GetFormattedName(proto), fmt.GetFormattedName(proto), arg.GetFormattedName(proto)));
		else
			proto.codeLines.push_back(std::format("const char* {} = funcs->SNPrintF(inst, {}, {});", out.GetFormattedName(proto), fmt.GetFormattedName(proto), arg.GetFormattedName(proto)));
	};
	proto.codeElements.push_back(ele);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> SNPrintFNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<StringVariable>>("Format", ImFlow::ConnectionFilter::SameType(), StringVariable("%s")));
	info.push_back(std::make_shared<ImFlow::InPinProto<StringVariable>>("Arg", ImFlow::ConnectionFilter::SameType(), StringVariable("")));
	info.push_back(std::make_shared<ImFlow::OutPinProto<StringVariable>>("Value"));
	return info;
}

SetHiddenNode::SetHiddenNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {}

SetHiddenNode::SetHiddenNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):SetHiddenNode(rend,style) {}

void SetHiddenNode::draw() {}

void SetHiddenNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void SetHiddenNode::Export(RuiExportPrototype& proto) {
	BoolVariable cond = getInVal<BoolVariable>("Hidden");
	proto.AddDataVariable(IntVariable(cond.value, cond.name));
	ExportElement<std::string> ele;
#if _DEBUG
	ele.sourceNodeName = typeid(*this).name();
#endif
	ele.identifier = Variable::UniqueName();
	ele.dependencys = { cond.name };
	ele.callback = [cond](RuiExportPrototype& proto) {
		proto.codeLines.push_back(std::format("if({}) funcs->SetHidden(inst);", cond.GetFormattedName(proto)));
	};
	proto.codeElements.push_back(ele);
}

std::vector<std::shared_ptr<ImFlow::PinProto>> SetHiddenNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<BoolVariable>>("Hidden", ImFlow::ConnectionFilter::SameType(), BoolVariable(false)));
	return info;
}

void AddArgumentNodes(NodeEditor& editor) {
	editor.AddNodeType<IntArgNode>();
	editor.AddNodeType<BoolArgNode>();
	editor.AddNodeType<FloatArgNode>();
	editor.AddNodeType<GametimeArgNode>();
	editor.AddNodeType<WalltimeArgNode>();
	editor.AddNodeType<Float2ArgNode>();
	editor.AddNodeType<Float3ArgNode>();
	editor.AddNodeType<ColorArgNode>();
	editor.AddNodeType<StringArgNode>();
	editor.AddNodeType<AssetArgNode>();
	editor.AddNodeType<ImageArgNode>();
	editor.AddNodeType<UiHandleArgNode>();
	editor.AddNodeType<FontFaceArgNode>();
	editor.AddNodeType<FontHashArgNode>();
	editor.AddNodeType<ArrayArgNode>();
	editor.AddNodeType<LocalizeNode>();
	editor.AddNodeType<SNPrintFNode>();
	editor.AddNodeType<SetHiddenNode>();
}
