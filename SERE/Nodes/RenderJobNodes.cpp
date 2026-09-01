#include "RenderJobNodes.h"
#include "RuiRendering/RenderManager.h"
#include <array>

static void FillStyleHdr(StyleDescriptorOffsets& style, uint16_t type, RuiExportPrototype& proto,
	const ColorVariable& c0, const ColorVariable& c1, const ColorVariable& c2,
	const FloatVariable& blend, const FloatVariable& premul,
	const ColorVariable& tint, const FloatVariable& hue,
	const FloatVariable& sat, const FloatVariable& light)
{
	style.type = type;
	style.color0 = proto.GetColorDataVariableOffset(c0);
	style.color1 = proto.GetColorDataVariableOffset(c1);
	style.color2 = proto.GetColorDataVariableOffset(c2);
	style.blend = proto.GetFloatDataVariableOffset(blend);
	style.premul = proto.GetFloatDataVariableOffset(premul);
	ColorOffsets tintOff = proto.GetColorDataVariableOffset(tint);
	style.tint[0] = tintOff.red;
	style.tint[1] = tintOff.green;
	style.tint[2] = tintOff.blue;
	style.tint[3] = tintOff.alpha;
	style.hue = proto.GetFloatDataVariableOffset(hue);
	style.saturation = proto.GetFloatDataVariableOffset(sat);
	style.lightness = proto.GetFloatDataVariableOffset(light);
	style.kerning = proto.GetFloatConstantOffset(0.f);
}

#pragma pack(push, 1)
struct VideoWidgetOffsets {
	uint16_t type = 3;
	uint16_t visOff = 0;
	uint16_t xfrmIdx = 0;
	uint16_t clipXfrmIdx = 0;
	uint16_t videoChannel = 0;
	Float2Offsets clipMin{};
	Float2Offsets clipMax{};
	Float2Offsets uvMin{};
	Float2Offsets uvMax{};
	uint16_t opts = 0;
	uint8_t styleIdx = 0;
	uint8_t pad = 0;
};
struct NestedWidgetOffsets {
	uint16_t type = 5;
	uint16_t visOff = 0;
	uint16_t xfrmIdx = 0;
	uint16_t clipXfrmIdx = 0;
	uint16_t uiHandle = 0;
	uint16_t opts = 0;
	uint8_t styleIdx = 0;
	uint8_t pad = 0;
};
struct CameraWidgetOffsets {
	uint16_t type = 4;
	uint16_t visOff = 0;
	uint16_t xfrmIdx = 0;
	uint16_t clipXfrmIdx = 0;
	uint16_t slot = 0;
	uint16_t clipImage = 0;
	Float2Offsets mins{};
	Float2Offsets maxs{};
	Float2Offsets uvMin{};
	Float2Offsets uvMax{};
	Float2Offsets maskCenter{};
	Float2Offsets maskTranslate{};
	Float2Offsets maskSize{};
	uint16_t maskRotation = 0;
	uint16_t maskToggle = 0;
	uint16_t opts = 0;
	uint8_t styleIdx = 0;
	uint8_t pad = 0;
};
#pragma pack(pop)
static_assert(sizeof(VideoWidgetOffsets) == 30, "UiWidget_Video_s is 30");
static_assert(sizeof(NestedWidgetOffsets) == 14, "UiWidget_Nested_s is 14");
static_assert(sizeof(CameraWidgetOffsets) == 48, "camera widget is 48");







AssetRenderNode::AssetRenderNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style),maskFlag(false),layer(0) {

	getIn<TransformResult>("Transform")->setEmptyVal(render.transformResults[2]);
}

AssetRenderNode::AssetRenderNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj) :AssetRenderNode(rend, style) {
	if (obj.HasMember("Layer") && obj["Layer"].IsInt()) {
		layer = obj["Layer"].GetInt();
	}
}

void AssetRenderNode::draw() {
	ImGui::PushItemWidth(70.f);
	ImGui::Checkbox("Mask mode",&maskFlag);
	ImGui::InputInt("Layer",&layer);
	ImGui::PopItemWidth();
	AssetInputData input{};
	input.mainColor = getInVal<ColorVariable>("Main Color");
	input.maskColor = getInVal<ColorVariable>("Mask Color");
	input.tertColor = getInVal<ColorVariable>("Tertiary Color");
	input.mainAsset = getInVal<AssetVariable>("Main Asset");
	input.maskAsset = getInVal<AssetVariable>("Mask Asset");
	input.mins = getInVal<Float2Variable>("Mins");
	input.maxs = getInVal<Float2Variable>("Maxs");
	input.texMins = getInVal<Float2Variable>("Texture Mins");
	input.texMaxs = getInVal<Float2Variable>("Texture Maxs");
	input.blend = getInVal<FloatVariable>("Blend");
	input.premul = getInVal<FloatVariable>("Premul");
	input.tint = getInVal<ColorVariable>("Tint");
	input.hue = getInVal<FloatVariable>("Hue");
	input.saturation = getInVal<FloatVariable>("Saturation");
	input.lightness = getInVal<FloatVariable>("Lightness");
	input.maskCenter = getInVal<Float2Variable>("Mask Center");
	input.maskTranslate = getInVal<Float2Variable>("Mask Translate");
	input.maskSize = getInVal<Float2Variable>("Mask Size");
	input.maskRotation = getInVal<FloatVariable>("Mask Rotation");
	input.transform = getInVal<TransformResult>("Transform");
	input.flags = maskFlag ? 0x1001 : 0x1000;

	render.jobs.emplace_back(layer, [input](RenderInstance& render) {
		Render_Asset(render,input);
	} );
	
}

void AssetRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("Layer",layer,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void AssetRenderNode::Export(RuiExportPrototype& proto) {
	proto.renderJobCount++;
	// Ensure 0.0f and 1.0f constants exist for asset style descriptor defaults
	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	AssetInputData input{};
	input.mainColor = getInVal<ColorVariable>("Main Color");
	input.maskColor = getInVal<ColorVariable>("Mask Color");
	input.tertColor = getInVal<ColorVariable>("Tertiary Color");
	input.mainAsset = getInVal<AssetVariable>("Main Asset");
	input.maskAsset = getInVal<AssetVariable>("Mask Asset");
	input.mins = getInVal<Float2Variable>("Mins");
	input.maxs = getInVal<Float2Variable>("Maxs");
	input.texMins = getInVal<Float2Variable>("Texture Mins");
	input.texMaxs = getInVal<Float2Variable>("Texture Maxs");
	input.blend = getInVal<FloatVariable>("Blend");
	input.premul = getInVal<FloatVariable>("Premul");
	input.tint = getInVal<ColorVariable>("Tint");
	input.hue = getInVal<FloatVariable>("Hue");
	input.saturation = getInVal<FloatVariable>("Saturation");
	input.lightness = getInVal<FloatVariable>("Lightness");
	input.maskCenter = getInVal<Float2Variable>("Mask Center");
	input.maskTranslate = getInVal<Float2Variable>("Mask Translate");
	input.maskSize = getInVal<Float2Variable>("Mask Size");
	input.maskRotation = getInVal<FloatVariable>("Mask Rotation");
	input.transform = getInVal<TransformResult>("Transform");
	if (inPin("Clip")->isConnected())
		input.clipHash = getInVal<TransformResult>("Clip").hash;
	input.flags = maskFlag ? 0x1001 : 0x1000;


	if (!input.mainAsset.name.size()) {
		std::string mainAssetFallback = Variable::UniqueName();
		input.mainAsset.name = mainAssetFallback;
		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = typeid(*this).name();
#endif
		ele.dependencys = { };
		ele.identifier = mainAssetFallback;
		ele.callback =[mainAssetFallback](RuiExportPrototype& proto) {

			proto.codeLines.push_back(std::format("data->{} = funcs->LoadAsset(inst, data->{}, \"white\", 0ull);",mainAssetFallback,mainAssetFallback));
		};
		proto.codeElements.push_back(ele);
	}


	if (!input.maskAsset.name.size()) {
		std::string maskAssetFallback = Variable::UniqueName();
		input.maskAsset.name = maskAssetFallback;
		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = typeid(*this).name();
#endif
		ele.dependencys = { };
		ele.identifier = maskAssetFallback;
		ele.callback =[maskAssetFallback](RuiExportPrototype& proto) {
			
			proto.codeLines.push_back(std::format("data->{} = -1;",maskAssetFallback));
		};
		proto.codeElements.push_back(ele);
	}

	proto.AddDataVariable(input.mainColor);
	proto.AddDataVariable(input.maskColor);
	proto.AddDataVariable(input.tertColor);
	proto.AddDataVariable(input.mainAsset);
	proto.AddDataVariable(input.maskAsset);
	proto.AddDataVariable(input.mins);
	proto.AddDataVariable(input.maxs);
	proto.AddDataVariable(input.texMins);
	proto.AddDataVariable(input.texMaxs);
	proto.AddDataVariable(input.blend);
	proto.AddDataVariable(input.premul);
	proto.AddDataVariable(input.maskCenter);
	proto.AddDataVariable(input.maskTranslate);
	proto.AddDataVariable(input.maskSize);
	proto.AddDataVariable(input.maskRotation);
	proto.AddDataVariable(input.tint);
	proto.AddDataVariable(input.hue);
	proto.AddDataVariable(input.saturation);
	proto.AddDataVariable(input.lightness);

	proto.renderJobs.emplace_back(layer,[input](RuiExportPrototype& proto) {
		StyleDescriptorOffsets style{};
		style.type = 1;
		style.color0 = proto.GetColorDataVariableOffset(input.mainColor);
		style.color1 = proto.GetColorDataVariableOffset(input.maskColor);
		style.color2 = proto.GetColorDataVariableOffset(input.tertColor);
		style.blend = proto.GetFloatDataVariableOffset(input.blend);
		style.premul = proto.GetFloatDataVariableOffset(input.premul);
		// Set tint (multiplicative, 1.0 = no change) and HSL/kerning (additive, 0.0 = no change)
		uint16_t zeroOff = proto.GetFloatConstantOffset(0.f);
		ColorOffsets tintOff = proto.GetColorDataVariableOffset(input.tint);
		style.tint[0] = tintOff.red;
		style.tint[1] = tintOff.green;
		style.tint[2] = tintOff.blue;
		style.tint[3] = tintOff.alpha;
		style.hue = proto.GetFloatDataVariableOffset(input.hue);
		style.saturation = proto.GetFloatDataVariableOffset(input.saturation);
		style.lightness = proto.GetFloatDataVariableOffset(input.lightness);
		style.kerning = zeroOff;
		uint16_t styleId = proto.AddStyle(style);
		// Asset widget (50 bytes). hdr+0x02 is an offset into instance data:
		// engine culls the widget if (data[off] & 3) != 0, so this must land on a 0.0f.
		struct AssetRenderOffsets {
			uint16_t type = 1;           // +0x00
			uint16_t visOff = 0;         // +0x02 UiWidgetHdr_s.drawFlags
			uint16_t xfrmIdx;            // +0x04
			uint16_t clipXfrmIdx = 0;    // +0x06
			uint16_t mainAsset;          // +0x08
			uint16_t clipAsset;          // +0x0A
			Float2Offsets mins;          // +0x0C
			Float2Offsets maxs;          // +0x10
			Float2Offsets texMins;       // +0x14
			Float2Offsets texMaxs;       // +0x18
			Float2Offsets maskCenter;    // +0x1C
			uint16_t maskRotation;       // +0x20
			Float2Offsets maskTranslate; // +0x22
			Float2Offsets maskSize;      // +0x26
			uint16_t maskToggle;         // +0x2A
			uint16_t unk_2C = 0;         // +0x2C
			uint16_t flags;              // +0x2E
			uint8_t styleIdx;            // +0x30
			char pad = 0;                // +0x31
		};
		AssetRenderOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[input.transform.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.clipXfrmIdx = input.clipHash ? proto.transformIndices[input.clipHash] : 0;
		rend.mainAsset = proto.GetAssetDataVariableOffset(input.mainAsset);
		rend.clipAsset = proto.GetAssetDataVariableOffset(input.maskAsset);
		printf("[SERE] Asset widget: mainAsset name='%s' off=0x%04X, clipAsset name='%s' off=0x%04X\n",
			input.mainAsset.name.c_str(), rend.mainAsset, input.maskAsset.name.c_str(), rend.clipAsset);
		rend.mins = proto.GetFloat2DataVariableOffset(input.mins);
		rend.maxs = proto.GetFloat2DataVariableOffset(input.maxs);
		rend.texMins = proto.GetFloat2DataVariableOffset(input.texMins);
		rend.texMaxs = proto.GetFloat2DataVariableOffset(input.texMaxs);
		rend.maskCenter = proto.GetFloat2DataVariableOffset(input.maskCenter);
		rend.maskRotation = proto.GetFloatDataVariableOffset(input.maskRotation);
		rend.maskTranslate = proto.GetFloat2DataVariableOffset(input.maskTranslate);
		rend.maskSize = proto.GetFloat2DataVariableOffset(input.maskSize);
		rend.maskToggle = proto.GetFloatConstantOffset(0.f); // byte at 0.0f offset = 0x00 = no mask
		rend.flags = input.flags;
		rend.styleIdx = styleId;
		proto.AddRenderJobData((uint8_t*)& rend, sizeof(rend));

	});
}

std::vector<std::shared_ptr<ImFlow::PinProto>> AssetRenderNode::GetPinInfo() {
	// Built fresh per node. A static list hands every instance the SAME PinProto
	// objects, so two widgets of this type share pin state.
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Main Color",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Mask Color",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tertiary Color",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<AssetVariable>>("Main Asset",ImFlow::ConnectionFilter::SameType(),AssetVariable("white")));
	info.push_back(std::make_shared<ImFlow::InPinProto<AssetVariable>>("Mask Asset",ImFlow::ConnectionFilter::SameType(),AssetVariable("")));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mins",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Maxs",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Mins",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Maxs",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation",isPinNumeric,FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Center",ImFlow::ConnectionFilter::SameType(),Float2Variable(.5f,.5f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Translate",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Size",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Mask Rotation",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Transform",ImFlow::ConnectionFilter::SameType(),TransformResult()));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Clip",ImFlow::ConnectionFilter::SameType(), TransformResult()));
	return info;
}

AssetCircleRenderNode::AssetCircleRenderNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style),layer(0) {

	getIn<TransformResult>("Transform")->setEmptyVal(render.transformResults[2]);
}

AssetCircleRenderNode::AssetCircleRenderNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj) :AssetCircleRenderNode(rend, style) {
	if (obj.HasMember("Layer") && obj["Layer"].IsInt()) {
		layer = obj["Layer"].GetInt();
	}
}

void AssetCircleRenderNode::draw() {

	ImGui::PushItemWidth(70.f);
	ImGui::InputInt("Layer",&layer);
	ImGui::PopItemWidth();

	AssetCircleInputData input{};
	input.mainColor = getInVal<ColorVariable>("Main Color");
	input.scndColor = getInVal<ColorVariable>("Secondary Color");
	input.tertColor = getInVal<ColorVariable>("Tertiary Color");
	input.mainAsset = getInVal<AssetVariable>("Asset");
	input.blend = getInVal<FloatVariable>("Blend");
	input.premul = getInVal<FloatVariable>("Premul");
	input.tint = getInVal<ColorVariable>("Tint");
	input.hue = getInVal<FloatVariable>("Hue");
	input.saturation = getInVal<FloatVariable>("Saturation");
	input.lightness = getInVal<FloatVariable>("Lightness");
	input.mins = getInVal<Float2Variable>("Mins");
	input.maxs = getInVal<Float2Variable>("Maxs");
	input.texMins = getInVal<Float2Variable>("Texture Mins");
	input.texMaxs = getInVal<Float2Variable>("Texture Maxs");
	input.innerSliceBlend = getInVal<FloatVariable>("Inner Slice Blend");
	input.sliceBegin = getInVal<FloatVariable>("Slice Begin");
	input.sliceEnd = getInVal<FloatVariable>("Slice End");
	input.ellipseSize = getInVal<Float2Variable>("Ellipse Size");
	input.innerMask = getInVal<FloatVariable>("Inner Mask");
	input.vingette = getInVal<FloatVariable>("Vingette");

	input.transform = getInVal<TransformResult>("Transform");
	input.flags = 0x2000;

	render.jobs.emplace_back(layer, [input](RenderInstance& render) {
		Render_AssetSmall(render,input);
	});
	
}

void AssetCircleRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("Layer",layer,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void AssetCircleRenderNode::Export(RuiExportPrototype& proto) {
	proto.renderJobCount++;
	AssetCircleInputData input{};
	input.mainColor = getInVal<ColorVariable>("Main Color");
	input.scndColor = getInVal<ColorVariable>("Secondary Color");
	input.tertColor = getInVal<ColorVariable>("Tertiary Color");
	input.mainAsset = getInVal<AssetVariable>("Asset");
	input.blend = getInVal<FloatVariable>("Blend");
	input.premul = getInVal<FloatVariable>("Premul");
	input.tint = getInVal<ColorVariable>("Tint");
	input.hue = getInVal<FloatVariable>("Hue");
	input.saturation = getInVal<FloatVariable>("Saturation");
	input.lightness = getInVal<FloatVariable>("Lightness");
	input.mins = getInVal<Float2Variable>("Mins");
	input.maxs = getInVal<Float2Variable>("Maxs");
	input.texMins = getInVal<Float2Variable>("Texture Mins");
	input.texMaxs = getInVal<Float2Variable>("Texture Maxs");
	input.innerSliceBlend = getInVal<FloatVariable>("Inner Slice Blend");
	input.sliceBegin = getInVal<FloatVariable>("Slice Begin");
	input.sliceEnd = getInVal<FloatVariable>("Slice End");
	input.ellipseSize = getInVal<Float2Variable>("Ellipse Size");
	input.innerMask = getInVal<FloatVariable>("Inner Mask");
	input.vingette = getInVal<FloatVariable>("Vingette");

	input.transform = getInVal<TransformResult>("Transform");
	input.flags = 0x2000;

	// Ensure mainAsset has a named variable with -1 fallback if not connected
	if (!input.mainAsset.name.size()) {
		std::string assetFallback = Variable::UniqueName();
		input.mainAsset.name = assetFallback;
		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = typeid(*this).name();
#endif
		ele.dependencys = {};
		ele.identifier = assetFallback;
		ele.callback = [assetFallback](RuiExportPrototype& proto) {
			proto.codeLines.push_back(std::format("data->{} = -4;", assetFallback));
		};
		proto.codeElements.push_back(ele);
	}

	proto.AddDataVariable(input.mainColor);
	proto.AddDataVariable(input.scndColor);
	proto.AddDataVariable(input.tertColor);
	proto.AddDataVariable(input.mainAsset);
	proto.AddDataVariable(input.blend);
	proto.AddDataVariable(input.premul);
	proto.AddDataVariable(input.mins);
	proto.AddDataVariable(input.maxs);
	proto.AddDataVariable(input.texMins);
	proto.AddDataVariable(input.texMaxs);
	proto.AddDataVariable(input.innerSliceBlend);
	proto.AddDataVariable(input.sliceBegin);
	proto.AddDataVariable(input.sliceEnd);
	proto.AddDataVariable(input.ellipseSize);
	proto.AddDataVariable(input.innerMask);
	proto.AddDataVariable(input.vingette);
	proto.AddDataVariable(input.tint);
	proto.AddDataVariable(input.hue);
	proto.AddDataVariable(input.saturation);
	proto.AddDataVariable(input.lightness);

	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	proto.renderJobs.emplace_back(layer,[input](RuiExportPrototype& proto) {
		StyleDescriptorOffsets style{};
		style.type = 2;
		style.color0 = proto.GetColorDataVariableOffset(input.mainColor);
		style.color1 = proto.GetColorDataVariableOffset(input.scndColor);
		style.color2 = proto.GetColorDataVariableOffset(input.tertColor);
		style.blend = proto.GetFloatDataVariableOffset(input.blend);
		style.premul = proto.GetFloatDataVariableOffset(input.premul);
		// Set tint and HSL/kerning from input variables
		uint16_t zeroOff = proto.GetFloatConstantOffset(0.f);
		ColorOffsets tintOff = proto.GetColorDataVariableOffset(input.tint);
		style.tint[0] = tintOff.red;
		style.tint[1] = tintOff.green;
		style.tint[2] = tintOff.blue;
		style.tint[3] = tintOff.alpha;
		style.hue = proto.GetFloatDataVariableOffset(input.hue);
		style.saturation = proto.GetFloatDataVariableOffset(input.saturation);
		style.lightness = proto.GetFloatDataVariableOffset(input.lightness);
		style.kerning = zeroOff;
		// Ellipse-specific fields stored in font slots
		style.fontHash = proto.GetFloatDataVariableOffset(input.innerSliceBlend);
		style.shadowAlpha = proto.GetFloatDataVariableOffset(input.sliceBegin);
		style.shadowOffsetX = proto.GetFloatDataVariableOffset(input.sliceEnd);
		auto ellipseSizeOffset = proto.GetFloat2DataVariableOffset(input.ellipseSize);
		style.shadowOffsetY = ellipseSizeOffset.x;
		style.shadowBlur = ellipseSizeOffset.y;
		style.pixelHeight = proto.GetFloatDataVariableOffset(input.innerMask);
		style.pixelAspect = proto.GetFloatDataVariableOffset(input.vingette);
		uint8_t styleId = (uint8_t)proto.AddStyle(style);
		// Ellipse widget (30 bytes) — same as above struct
		// Ellipse widget (30 bytes)
		struct AssetCircleRenderOffsets {
			uint16_t type = 2;           // +0x00
			uint16_t visOff = 0;         // +0x02 UiWidgetHdr_s.drawFlags
			uint16_t xfrmIdx;            // +0x04
			uint16_t clipXfrmIdx = 0;    // +0x06
			uint16_t mainAsset;          // +0x08
			Float2Offsets mins;          // +0x0A
			Float2Offsets maxs;          // +0x0E
			Float2Offsets texMins;       // +0x12
			Float2Offsets texMaxs;       // +0x16
			uint16_t flags;              // +0x1A
			uint8_t styleIdx;            // +0x1C
			char pad = 0;                // +0x1D
		};
		AssetCircleRenderOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[input.transform.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.clipXfrmIdx = 0;
		rend.mainAsset = proto.GetAssetDataVariableOffset(input.mainAsset);
		rend.mins = proto.GetFloat2DataVariableOffset(input.mins);
		rend.maxs = proto.GetFloat2DataVariableOffset(input.maxs);
		rend.texMins = proto.GetFloat2DataVariableOffset(input.texMins);
		rend.texMaxs = proto.GetFloat2DataVariableOffset(input.texMaxs);
		rend.flags = input.flags;
		rend.styleIdx = styleId;
		proto.AddRenderJobData((uint8_t*)&rend, sizeof(rend));

	});
}

std::vector<std::shared_ptr<ImFlow::PinProto>> AssetCircleRenderNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Main Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Secondary Color",ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tertiary Color",ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<AssetVariable>>("Asset", ImFlow::ConnectionFilter::SameType(), AssetVariable("white")));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation",isPinNumeric,FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mins",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Maxs",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Mins",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Maxs",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Inner Slice Blend",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Slice Begin",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Slice End",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Ellipse Size",ImFlow::ConnectionFilter::SameType(),Float2Variable(1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Inner Mask",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Vingette",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Transform",ImFlow::ConnectionFilter::SameType(),TransformResult()));
	return info;
}



static Font_t* FirstAvailableFont() {
	for (auto& atlas : fonts) {
		if (!atlas.fonts.empty())
			return &atlas.fonts.begin()->second;
	}
	return nullptr;
}

TextStyleNode::TextStyleNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {

	// Never hold dangling pointers into fonts after ReloadAssets; resolve each tick.
	currentFont = FirstAvailableFont();

	getOut<TextStyleData>("Style")->behaviour([this]() {
		TextStyleData res;
		res.mainColor = getInVal<ColorVariable>("mainColor");
		res.scndColor = getInVal<ColorVariable>("scndColor");
		res.tertColor = getInVal<ColorVariable>("tertColor");
		res.blend = getInVal<FloatVariable>("Blend");
		res.premul = getInVal<FloatVariable>("Premul");
		res.tint = getInVal<ColorVariable>("Tint");
		res.hue = getInVal<FloatVariable>("Hue");
		res.saturation = getInVal<FloatVariable>("Saturation");
		res.lightness = getInVal<FloatVariable>("Lightness");
		res.kerning = getInVal<FloatVariable>("Kerning");
		res.dropShadowHardness = getInVal<FloatVariable>("Dropshadow Hardness");
		res.dropShadowOffset = getInVal<Float2Variable>("Dropshadow Offset");
		res.dropShadowBlur = getInVal<FloatVariable>("Dropshadow Blur");
		Font_t* f = currentFont ? currentFont : FirstAvailableFont();
		res.fontIndex = f ? f->fontIndex : 0;
		res.size = getInVal<FloatVariable>("Size");
		res.stretchX = getInVal<FloatVariable>("stretchX");
		res.backgroundSize = getInVal<FloatVariable>("backgroundSize");
		res.boltness = getInVal<FloatVariable>("Boltness");
		res.blur = getInVal<FloatVariable>("Blur");
		res.style_32 = getInVal<FloatVariable>("style_32");
		return res;
	});
}

TextStyleNode::TextStyleNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj) :TextStyleNode(rend, style) {
	if (obj.HasMember("FontName") && obj["FontName"].IsString()) {
		const char* fontName = obj["FontName"].GetString();
		// Engine resolves face by GetFontIndex(StringToGuid(name)), not rpak string equality.
		currentFont = getFontByName(fontName);
		if (!currentFont)
			currentFont = FirstAvailableFont();
		printf("[SERE] TextStyle: name='%s' face=%u resolved=%s\n",
			fontName,
			(unsigned)FontFaceIndexFromName(fontName),
			currentFont ? currentFont->name.c_str() : "(none)");
		fflush(stdout);
	}
	
}

void TextStyleNode::draw() {
	ImGui::PushItemWidth(130.f);
	if (!currentFont)
		currentFont = FirstAvailableFont();
	const char* label = currentFont ? currentFont->name.c_str() : "(no fonts)";
	if (ImGui::BeginCombo("Font", label)) {
		for (auto& atlas : fonts) {
			for (auto& [index,font] : atlas.fonts) {
				bool isSelected = currentFont && index == currentFont->fontIndex;
				if (ImGui::Selectable(font.name.c_str(), isSelected)) {
					currentFont = &font;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void TextStyleNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	const char* fontName = (currentFont ? currentFont->name.c_str() : "");
	obj.AddMember("FontName", rapidjson::Value(fontName, allocator), allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void TextStyleNode::Export(RuiExportPrototype&){}//export handled in TextRender

std::vector<std::shared_ptr<ImFlow::PinProto>> TextStyleNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("mainColor",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("scndColor",ImFlow::ConnectionFilter::SameType(),ColorVariable(0.f,0.f,0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("tertColor",ImFlow::ConnectionFilter::SameType(),ColorVariable(0.f,0.f,0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint",ImFlow::ConnectionFilter::SameType(),ColorVariable(1.f,1.f,1.f,1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation",isPinNumeric,FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Kerning",isPinNumeric,FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Dropshadow Hardness",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Dropshadow Offset",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Dropshadow Blur",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Size",ImFlow::ConnectionFilter::SameType(),FloatVariable(10.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("stretchX",ImFlow::ConnectionFilter::SameType(),FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("backgroundSize",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Boltness",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blur",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("style_32",ImFlow::ConnectionFilter::SameType(),FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::OutPinProto<TextStyleData>>("Style"));
	return info;
}

TextSizeNode::TextSizeNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style) {
	std::string sizeName = Variable::UniqueName();
	getOut<TextInputData>("Text Data")->behaviour([this,sizeName]() {
		
		TextInputData data;

		data.text = getInVal<StringVariable>("text");
		data.minSize = getInVal<Float2Variable>("minSize");
		data.maxSize = getInVal<Float2Variable>("maxSize");
		data.styles[0] = getInVal<TextStyleData>("Style_0");
		data.styles[1] = getInVal<TextStyleData>("Style_1");
		data.styles[2] = getInVal<TextStyleData>("Style_2");
		data.styles[3] = getInVal<TextStyleData>("Style_3");
		for (int i = 0; i < 4; i++)
			data.styleDriven[i] = i == 0 || inPin(std::format("Style_{}", i))->isConnected();
		data.sizeName = sizeName;
		GetTextSize(data);
		return data;
	});
	getOut<TransformSize>("Size")->behaviour([this,sizeName]() {
		
		TextInputData data;

		data.text = getInVal<StringVariable>("text");
		data.minSize = getInVal<Float2Variable>("minSize");
		data.maxSize = getInVal<Float2Variable>("maxSize");
		data.styles[0] = getInVal<TextStyleData>("Style_0");
		data.styles[1] = getInVal<TextStyleData>("Style_1");
		data.styles[2] = getInVal<TextStyleData>("Style_2");
		data.styles[3] = getInVal<TextStyleData>("Style_3");
		for (int i = 0; i < 4; i++)
			data.styleDriven[i] = i == 0 || inPin(std::format("Style_{}", i))->isConnected();
		
		TransformSize size{ GetTextSize(data),sizeName };
		return size;
	});

}

TextSizeNode::TextSizeNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj) :TextSizeNode(rend, style) {}

void TextSizeNode::draw() {


}

void TextSizeNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}


void TextSizeNode::Export(RuiExportPrototype&){}

std::vector<std::shared_ptr<ImFlow::PinProto>> TextSizeNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<StringVariable>>("text",ImFlow::ConnectionFilter::SameType(),StringVariable("Default Text")));

	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("minSize",ImFlow::ConnectionFilter::SameType(),Float2Variable(0.f,0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("maxSize",ImFlow::ConnectionFilter::SameType(),Float2Variable(1000000000.f,1000000000.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TextStyleData>>("Style_0",ImFlow::ConnectionFilter::SameType(),TextStyleData()));
	info.push_back(std::make_shared<ImFlow::InPinProto<TextStyleData>>("Style_1",ImFlow::ConnectionFilter::SameType(),TextStyleData()));
	info.push_back(std::make_shared<ImFlow::InPinProto<TextStyleData>>("Style_2",ImFlow::ConnectionFilter::SameType(),TextStyleData()));
	info.push_back(std::make_shared<ImFlow::InPinProto<TextStyleData>>("Style_3",ImFlow::ConnectionFilter::SameType(),TextStyleData()));
	info.push_back(std::make_shared<ImFlow::OutPinProto<TextInputData>>("Text Data"));
	info.push_back(std::make_shared<ImFlow::OutPinProto<TransformSize>>("Size"));
	return info;
}


TextRenderNode::TextRenderNode(RenderInstance& rend,ImFlow::StyleManager& style):RuiBaseNode(name,category,GetPinInfo(),rend,style),layer(0) {

	getIn<TransformResult>("Parent")->setEmptyVal(render.transformResults[2]);
}

TextRenderNode::TextRenderNode(RenderInstance& rend,ImFlow::StyleManager& style, rapidjson::GenericObject<false,rapidjson::Value> obj):TextRenderNode(rend,style){
	if (obj.HasMember("Layer") && obj["Layer"].IsInt()) {
		layer = obj["Layer"].GetInt();
	}
}

void TextRenderNode::draw() {
	const TextInputData& data = getInVal<TextInputData>("Data");
	const TransformResult& parent = getInVal<TransformResult>("Parent");
	ImGui::PushItemWidth(70.f);
	ImGui::InputInt("Layer",&layer);
	ImGui::PopItemWidth();
	render.jobs.emplace_back(layer, [data, parent](RenderInstance& render) {
		Text_Render(render,data,parent);
	});
}

void TextRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name",name,allocator);
	obj.AddMember("Category",category,allocator);
	obj.AddMember("Layer",layer,allocator);
	RuiBaseNode::Serialize(obj,allocator);
}

void AddTextStyleVariablesToProto(RuiExportPrototype& proto,const TextStyleData& style) {
	proto.AddDataVariable(style.mainColor);
	proto.AddDataVariable(style.scndColor);
	proto.AddDataVariable(style.tertColor);
	proto.AddDataVariable(style.blend);
	proto.AddDataVariable(style.premul);
	proto.AddDataVariable(style.dropShadowBlur);
	proto.AddDataVariable(style.dropShadowOffset);
	proto.AddDataVariable(style.dropShadowHardness);
	proto.AddDataVariable(style.size);
	proto.AddDataVariable(style.stretchX);
	proto.AddDataVariable(style.backgroundSize);
	proto.AddDataVariable(style.boltness);
	proto.AddDataVariable(style.blur);
	proto.AddDataVariable(style.style_32);
	proto.AddDataVariable(style.tint);
	proto.AddDataVariable(style.hue);
	proto.AddDataVariable(style.saturation);
	proto.AddDataVariable(style.lightness);
	proto.AddDataVariable(style.kerning);
}

void AddTextStyleToDependency(std::set<std::string>& deps,const TextStyleData& style) {
	deps.insert(style.mainColor.name);
	deps.insert(style.scndColor.name);
	deps.insert(style.tertColor.name);
	deps.insert(style.blend.name);
	deps.insert(style.premul.name);
	deps.insert(style.dropShadowBlur.name);
	deps.insert(style.dropShadowOffset.name);
	deps.insert(style.dropShadowHardness.name);
	deps.insert(style.size.name);
	deps.insert(style.stretchX.name);
	deps.insert(style.backgroundSize.name);
	deps.insert(style.boltness.name);
	deps.insert(style.blur.name);
	deps.insert(style.style_32.name);
	deps.insert(style.tint.name);
	deps.insert(style.hue.name);
	deps.insert(style.saturation.name);
	deps.insert(style.lightness.name);
	deps.insert(style.kerning.name);
}

uint8_t AddTextStyleToProto(RuiExportPrototype& proto, const TextStyleData& style, const std::string& fontHashVarName) {

	StyleDescriptorOffsets style0{};
	style0.type = 0;
	style0.color0 = proto.GetColorDataVariableOffset(style.mainColor);
	style0.color1 = proto.GetColorDataVariableOffset(style.scndColor);
	style0.color2 = proto.GetColorDataVariableOffset(style.tertColor);
	style0.blend = proto.GetFloatDataVariableOffset(style.blend);
	style0.premul = proto.GetFloatDataVariableOffset(style.premul);
	style0.fontHash = proto.varOffsets[fontHashVarName]; // dataStruct offset to uint32 font hash
	Float2Offsets dropShadowOffset = proto.GetFloat2DataVariableOffset(style.dropShadowOffset);
	style0.shadowAlpha = proto.GetFloatDataVariableOffset(style.dropShadowHardness);
	style0.shadowOffsetX = dropShadowOffset.x;
	style0.shadowOffsetY = dropShadowOffset.y;
	style0.shadowBlur = proto.GetFloatDataVariableOffset(style.dropShadowBlur);
	style0.pixelHeight = proto.GetFloatDataVariableOffset(style.size);
	style0.pixelAspect = proto.GetFloatDataVariableOffset(style.stretchX);
	style0.outlineWidth = proto.GetFloatDataVariableOffset(style.backgroundSize);
	style0.thicken = proto.GetFloatDataVariableOffset(style.boltness);
	style0.blur = proto.GetFloatDataVariableOffset(style.blur);
	style0.baselineShift = proto.GetFloatDataVariableOffset(style.style_32);
	style0.kerning = proto.GetFloatDataVariableOffset(style.kerning);
	style0.hue = proto.GetFloatDataVariableOffset(style.hue);
	style0.saturation = proto.GetFloatDataVariableOffset(style.saturation);
	style0.lightness = proto.GetFloatDataVariableOffset(style.lightness);
	ColorOffsets tintOff = proto.GetColorDataVariableOffset(style.tint);
	style0.tint[0] = tintOff.red;
	style0.tint[1] = tintOff.green;
	style0.tint[2] = tintOff.blue;
	style0.tint[3] = tintOff.alpha;
	uint8_t res = (uint8_t)proto.AddStyle(style0);
	return res;
}

void TextRenderNode::Export(RuiExportPrototype& proto) {
	const TextInputData& input = getInVal<TextInputData>("Data");
	const TransformResult& parent = getInVal<TransformResult>("Parent");

	proto.renderJobCount++;
	proto.AddDataVariable(input.text);
	proto.AddDataVariable(input.minSize);
	proto.AddDataVariable(input.maxSize);
	// Register a large constant for lineBreakWidth.
	// minSize.x default is 0.0 which means "break after every character" in engine,
	// causing vertical text. We use 1e9 to mean "no line breaking".
	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	proto.AddConstant(500.f);
	proto.AddConstant(1000000000.f);
	for (int i = 0; i < 4; i++) {
		if (input.styleDriven[i])
			AddTextStyleVariablesToProto(proto,input.styles[i]);
	}

	// Allocate font index variables (uint32 each) in dataStruct for each text style slot.
	// The style descriptor fontHash field is a uint16 offset into the dataStruct where the
	// engine reads a uint16 font index (direct index into s_fontDataArray, max 256).
	// The DLL code writes the font index directly (no StringToHash needed).
	std::array<std::string, 4> fontHashNames;
	for (int i = 0; i < 4; i++) {
		if (!input.styleDriven[i]) continue;
		fontHashNames[i] = Variable::UniqueName();
		proto.varsInDataStruct.emplace(fontHashNames[i], VariableType::ASSET_HANDLE);

		std::string varName = fontHashNames[i];
		int fontIdx = input.styles[i].fontIndex;
		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = "TextRenderNode_FontHash";
#endif
		ele.dependencys = {};
		ele.identifier = varName;
		ele.callback = [varName, fontIdx](RuiExportPrototype& proto) {
			proto.codeLines.push_back(std::format("data->{} = {};", varName, fontIdx));
		};
		proto.codeElements.push_back(ele);
	}

	proto.renderJobs.emplace_back(layer,[input,parent,fontHashNames](RuiExportPrototype& proto) {

		// Text widget (28 bytes). visOff must point at a 0.0f (engine culls if data[off]&3).
		struct TextRenderOffsets {
			uint16_t type = 0;              // +0x00
			uint16_t visOff = 0;            // +0x02 UiWidgetHdr_s.drawFlags
			uint16_t xfrmIdx;               // +0x04
			uint16_t clipXfrmIdx = 0;       // +0x06
			uint8_t fontStyleIdx[4];        // +0x08
			uint16_t text;                  // +0x0C
			uint16_t fitToWidth;            // +0x0E
			uint16_t fitToHeight;           // +0x10
			uint16_t squishShrinkRatio;     // +0x12
			uint16_t lineBreakWidth;        // +0x14
			uint16_t textAlign;             // +0x16
			uint16_t locDir = 0;            // +0x18
			uint16_t leading = 0;           // +0x1A
		};
		TextRenderOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[parent.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.clipXfrmIdx = 0;
		// UI_GetTextSize looks up qword_142CE7118[style.fontHash] for all four slots
		// before it reads a single character, so an undriven slot pointed at face 0
		// dereferences a font that was never loaded. Point it at slot 0 instead.
		rend.fontStyleIdx[0] = AddTextStyleToProto(proto, input.styles[0], fontHashNames[0]);
		for (int i = 1; i < 4; i++) {
			rend.fontStyleIdx[i] = input.styleDriven[i]
				? AddTextStyleToProto(proto, input.styles[i], fontHashNames[i])
				: rend.fontStyleIdx[0];
		}
		rend.text = proto.GetStringDataVariableOffset(input.text);
		rend.fitToWidth = proto.GetFloatConstantOffset(500.f);
		rend.fitToHeight = proto.GetFloatConstantOffset(500.f);
		rend.squishShrinkRatio = proto.GetFloatConstantOffset(1.f);
		auto minSz = proto.GetFloat2DataVariableOffset(input.minSize);
		rend.lineBreakWidth = proto.GetFloatConstantOffset(1000000000.f);
		rend.textAlign = minSz.y;
		size_t jobOffset = proto.renderJobData.size();
		proto.AddRenderJobData((uint8_t*)&rend, sizeof(rend));

		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = "TextRenderNode";
#endif
		ele.identifier = input.sizeName;
		ele.dependencys = {input.text.name,input.minSize.name,input.maxSize.name};
		for (int i = 0; i < 4; i++)
			ele.dependencys.insert(fontHashNames[i]);
		AddTextStyleToDependency(ele.dependencys,input.styles[0]);
		AddTextStyleToDependency(ele.dependencys,input.styles[1]);
		AddTextStyleToDependency(ele.dependencys,input.styles[2]);
		AddTextStyleToDependency(ele.dependencys,input.styles[3]);
		ele.callback = [input, jobOffset](RuiExportPrototype& proto) {
			const uint16_t cacheIdx = proto.textCacheIdx++;
			proto.codeLines.push_back(std::format("__m128 {} = funcs->GetTextSize(inst,{},{});",input.sizeName,jobOffset,cacheIdx));
		};
		proto.codeElements.push_back(ele);

	});
}


std::vector<std::shared_ptr<ImFlow::PinProto>> TextRenderNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<TextInputData>>("Data",ImFlow::ConnectionFilter::SameType(),TextInputData()));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Parent",ImFlow::ConnectionFilter::SameType(),TransformResult()));
	return info;
}



static AssetInputData PreviewAssetFromTransform(const TransformResult& xfrm) {
	AssetInputData input{};
	input.transform = xfrm;
	input.flags = 0x1000;
	return input;
}

VideoRenderNode::VideoRenderNode(RenderInstance& rend, ImFlow::StyleManager& style)
	:RuiBaseNode(name, category, GetPinInfo(), rend, style), layer(0), opts(0) {
	getIn<TransformResult>("Transform")->setEmptyVal(render.transformResults[2]);
}

VideoRenderNode::VideoRenderNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj)
	:VideoRenderNode(rend, style) {
	if (obj.HasMember("Layer") && obj["Layer"].IsInt())
		layer = obj["Layer"].GetInt();
	if (obj.HasMember("Opts") && obj["Opts"].IsInt())
		opts = obj["Opts"].GetInt();
}

void VideoRenderNode::draw() {
	ImGui::PushItemWidth(70.f);
	ImGui::InputInt("Layer", &layer);
	ImGui::InputInt("Opts", &opts);
	ImGui::PopItemWidth();
	AssetInputData preview = PreviewAssetFromTransform(getInVal<TransformResult>("Transform"));
	render.jobs.emplace_back(layer, [preview](RenderInstance& rend) {
		Render_Asset(rend, preview);
	});
}

void VideoRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name", name, allocator);
	obj.AddMember("Category", category, allocator);
	obj.AddMember("Layer", layer, allocator);
	obj.AddMember("Opts", opts, allocator);
	RuiBaseNode::Serialize(obj, allocator);
}

void VideoRenderNode::Export(RuiExportPrototype& proto) {
	proto.renderJobCount++;
	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	IntVariable channel = getInVal<IntVariable>("Channel");
	ColorVariable mainColor = getInVal<ColorVariable>("Main Color");
	ColorVariable maskColor = getInVal<ColorVariable>("Mask Color");
	ColorVariable tertColor = getInVal<ColorVariable>("Tertiary Color");
	Float2Variable mins = getInVal<Float2Variable>("Mins");
	Float2Variable maxs = getInVal<Float2Variable>("Maxs");
	Float2Variable texMins = getInVal<Float2Variable>("Texture Mins");
	Float2Variable texMaxs = getInVal<Float2Variable>("Texture Maxs");
	FloatVariable blend = getInVal<FloatVariable>("Blend");
	FloatVariable premul = getInVal<FloatVariable>("Premul");
	ColorVariable tint = getInVal<ColorVariable>("Tint");
	FloatVariable hue = getInVal<FloatVariable>("Hue");
	FloatVariable saturation = getInVal<FloatVariable>("Saturation");
	FloatVariable lightness = getInVal<FloatVariable>("Lightness");
	TransformResult xfrm = getInVal<TransformResult>("Transform");
	int packedOpts = opts;
	proto.AddDataVariable(channel);
	proto.AddDataVariable(mainColor);
	proto.AddDataVariable(maskColor);
	proto.AddDataVariable(tertColor);
	proto.AddDataVariable(mins);
	proto.AddDataVariable(maxs);
	proto.AddDataVariable(texMins);
	proto.AddDataVariable(texMaxs);
	proto.AddDataVariable(blend);
	proto.AddDataVariable(premul);
	proto.AddDataVariable(tint);
	proto.AddDataVariable(hue);
	proto.AddDataVariable(saturation);
	proto.AddDataVariable(lightness);
	proto.renderJobs.emplace_back(layer, [channel, mainColor, maskColor, tertColor, mins, maxs, texMins, texMaxs,
		blend, premul, tint, hue, saturation, lightness, xfrm, packedOpts](RuiExportPrototype& proto) {
		StyleDescriptorOffsets style{};
		FillStyleHdr(style, 3, proto, mainColor, maskColor, tertColor, blend, premul, tint, hue, saturation, lightness);
		uint8_t styleId = (uint8_t)proto.AddStyle(style);
		VideoWidgetOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[xfrm.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.videoChannel = proto.GetIntDataVariableOffset(channel);
		rend.clipMin = proto.GetFloat2DataVariableOffset(mins);
		rend.clipMax = proto.GetFloat2DataVariableOffset(maxs);
		rend.uvMin = proto.GetFloat2DataVariableOffset(texMins);
		rend.uvMax = proto.GetFloat2DataVariableOffset(texMaxs);
		rend.opts = (uint16_t)packedOpts;
		rend.styleIdx = styleId;
		proto.AddRenderJobData((uint8_t*)&rend, sizeof(rend));
	});
}

std::vector<std::shared_ptr<ImFlow::PinProto>> VideoRenderNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<IntVariable>>("Channel", ImFlow::ConnectionFilter::SameType(), IntVariable(0)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Main Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Mask Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tertiary Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend", ImFlow::ConnectionFilter::SameType(), FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul", ImFlow::ConnectionFilter::SameType(), FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation", isPinNumeric, FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mins", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.f, 0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Maxs", ImFlow::ConnectionFilter::SameType(), Float2Variable(1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Mins", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.f, 0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Maxs", ImFlow::ConnectionFilter::SameType(), Float2Variable(1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Transform", ImFlow::ConnectionFilter::SameType(), TransformResult()));
	return info;
}

CameraRenderNode::CameraRenderNode(RenderInstance& rend, ImFlow::StyleManager& style)
	:RuiBaseNode(name, category, GetPinInfo(), rend, style), layer(0), opts(0) {
	getIn<TransformResult>("Transform")->setEmptyVal(render.transformResults[2]);
}

CameraRenderNode::CameraRenderNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj)
	:CameraRenderNode(rend, style) {
	if (obj.HasMember("Layer") && obj["Layer"].IsInt())
		layer = obj["Layer"].GetInt();
	if (obj.HasMember("Opts") && obj["Opts"].IsInt())
		opts = obj["Opts"].GetInt();
}

void CameraRenderNode::draw() {
	ImGui::PushItemWidth(70.f);
	ImGui::InputInt("Layer", &layer);
	ImGui::InputInt("Opts", &opts);
	ImGui::PopItemWidth();
	AssetInputData preview = PreviewAssetFromTransform(getInVal<TransformResult>("Transform"));
	render.jobs.emplace_back(layer, [preview](RenderInstance& rend) {
		Render_Asset(rend, preview);
	});
}

void CameraRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name", name, allocator);
	obj.AddMember("Category", category, allocator);
	obj.AddMember("Layer", layer, allocator);
	obj.AddMember("Opts", opts, allocator);
	RuiBaseNode::Serialize(obj, allocator);
}

void CameraRenderNode::Export(RuiExportPrototype& proto) {
	proto.renderJobCount++;
	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	IntVariable slot = getInVal<IntVariable>("Slot");
	AssetVariable clipImage = getInVal<AssetVariable>("Clip Image");
	ColorVariable mainColor = getInVal<ColorVariable>("Main Color");
	ColorVariable maskColor = getInVal<ColorVariable>("Mask Color");
	ColorVariable tertColor = getInVal<ColorVariable>("Tertiary Color");
	Float2Variable mins = getInVal<Float2Variable>("Mins");
	Float2Variable maxs = getInVal<Float2Variable>("Maxs");
	Float2Variable texMins = getInVal<Float2Variable>("Texture Mins");
	Float2Variable texMaxs = getInVal<Float2Variable>("Texture Maxs");
	Float2Variable maskCenter = getInVal<Float2Variable>("Mask Center");
	Float2Variable maskTranslate = getInVal<Float2Variable>("Mask Translate");
	Float2Variable maskSize = getInVal<Float2Variable>("Mask Size");
	FloatVariable maskRotation = getInVal<FloatVariable>("Mask Rotation");
	FloatVariable blend = getInVal<FloatVariable>("Blend");
	FloatVariable premul = getInVal<FloatVariable>("Premul");
	ColorVariable tint = getInVal<ColorVariable>("Tint");
	FloatVariable hue = getInVal<FloatVariable>("Hue");
	FloatVariable saturation = getInVal<FloatVariable>("Saturation");
	FloatVariable lightness = getInVal<FloatVariable>("Lightness");
	TransformResult xfrm = getInVal<TransformResult>("Transform");
	int packedOpts = opts;

	if (!clipImage.name.size()) {
		std::string clipFallback = Variable::UniqueName();
		clipImage.name = clipFallback;
		ExportElement<std::string> ele;
#if _DEBUG
		ele.sourceNodeName = typeid(*this).name();
#endif
		ele.dependencys = {};
		ele.identifier = clipFallback;
		ele.callback = [clipFallback](RuiExportPrototype& proto) {
			proto.codeLines.push_back(std::format("data->{} = -1;", clipFallback));
		};
		proto.codeElements.push_back(ele);
	}

	proto.AddDataVariable(slot);
	proto.AddDataVariable(clipImage);
	proto.AddDataVariable(mainColor);
	proto.AddDataVariable(maskColor);
	proto.AddDataVariable(tertColor);
	proto.AddDataVariable(mins);
	proto.AddDataVariable(maxs);
	proto.AddDataVariable(texMins);
	proto.AddDataVariable(texMaxs);
	proto.AddDataVariable(maskCenter);
	proto.AddDataVariable(maskTranslate);
	proto.AddDataVariable(maskSize);
	proto.AddDataVariable(maskRotation);
	proto.AddDataVariable(blend);
	proto.AddDataVariable(premul);
	proto.AddDataVariable(tint);
	proto.AddDataVariable(hue);
	proto.AddDataVariable(saturation);
	proto.AddDataVariable(lightness);

	proto.renderJobs.emplace_back(layer, [slot, clipImage, mainColor, maskColor, tertColor, mins, maxs, texMins, texMaxs,
		maskCenter, maskTranslate, maskSize, maskRotation, blend, premul, tint, hue, saturation, lightness, xfrm, packedOpts](RuiExportPrototype& proto) {
		StyleDescriptorOffsets style{};
		FillStyleHdr(style, 4, proto, mainColor, maskColor, tertColor, blend, premul, tint, hue, saturation, lightness);
		uint8_t styleId = (uint8_t)proto.AddStyle(style);
		CameraWidgetOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[xfrm.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.slot = proto.GetIntDataVariableOffset(slot);
		rend.clipImage = proto.GetAssetDataVariableOffset(clipImage);
		rend.mins = proto.GetFloat2DataVariableOffset(mins);
		rend.maxs = proto.GetFloat2DataVariableOffset(maxs);
		rend.uvMin = proto.GetFloat2DataVariableOffset(texMins);
		rend.uvMax = proto.GetFloat2DataVariableOffset(texMaxs);
		rend.maskCenter = proto.GetFloat2DataVariableOffset(maskCenter);
		rend.maskTranslate = proto.GetFloat2DataVariableOffset(maskTranslate);
		rend.maskSize = proto.GetFloat2DataVariableOffset(maskSize);
		rend.maskRotation = proto.GetFloatDataVariableOffset(maskRotation);
		rend.maskToggle = proto.GetFloatConstantOffset(0.f);
		rend.opts = (uint16_t)packedOpts;
		rend.styleIdx = styleId;
		proto.AddRenderJobData((uint8_t*)&rend, sizeof(rend));
	});
}

std::vector<std::shared_ptr<ImFlow::PinProto>> CameraRenderNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<IntVariable>>("Slot", ImFlow::ConnectionFilter::SameType(), IntVariable(0)));
	info.push_back(std::make_shared<ImFlow::InPinProto<AssetVariable>>("Clip Image", ImFlow::ConnectionFilter::SameType(), AssetVariable("")));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Main Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Mask Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tertiary Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend", ImFlow::ConnectionFilter::SameType(), FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul", ImFlow::ConnectionFilter::SameType(), FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation", isPinNumeric, FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mins", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.f, 0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Maxs", ImFlow::ConnectionFilter::SameType(), Float2Variable(1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Mins", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.f, 0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Texture Maxs", ImFlow::ConnectionFilter::SameType(), Float2Variable(1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Center", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.5f, 0.5f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Translate", ImFlow::ConnectionFilter::SameType(), Float2Variable(0.f, 0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<Float2Variable>>("Mask Size", ImFlow::ConnectionFilter::SameType(), Float2Variable(1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Mask Rotation", ImFlow::ConnectionFilter::SameType(), FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Transform", ImFlow::ConnectionFilter::SameType(), TransformResult()));
	return info;
}

NestedRenderNode::NestedRenderNode(RenderInstance& rend, ImFlow::StyleManager& style)
	:RuiBaseNode(name, category, GetPinInfo(), rend, style), layer(0), opts(0) {
	getIn<TransformResult>("Transform")->setEmptyVal(render.transformResults[2]);
}

NestedRenderNode::NestedRenderNode(RenderInstance& rend, ImFlow::StyleManager& style, rapidjson::GenericObject<false, rapidjson::Value> obj)
	:NestedRenderNode(rend, style) {
	if (obj.HasMember("Layer") && obj["Layer"].IsInt())
		layer = obj["Layer"].GetInt();
	if (obj.HasMember("Opts") && obj["Opts"].IsInt())
		opts = obj["Opts"].GetInt();
}

void NestedRenderNode::draw() {
	ImGui::PushItemWidth(70.f);
	ImGui::InputInt("Layer", &layer);
	ImGui::InputInt("Opts", &opts);
	ImGui::PopItemWidth();
	AssetInputData preview = PreviewAssetFromTransform(getInVal<TransformResult>("Transform"));
	render.jobs.emplace_back(layer, [preview](RenderInstance& rend) {
		Render_Asset(rend, preview);
	});
}

void NestedRenderNode::Serialize(rapidjson::GenericValue<rapidjson::UTF8<>>& obj, rapidjson::Document::AllocatorType& allocator) {
	obj.AddMember("Name", name, allocator);
	obj.AddMember("Category", category, allocator);
	obj.AddMember("Layer", layer, allocator);
	obj.AddMember("Opts", opts, allocator);
	RuiBaseNode::Serialize(obj, allocator);
}

void NestedRenderNode::Export(RuiExportPrototype& proto) {
	proto.renderJobCount++;
	proto.AddConstant(0.f);
	proto.AddConstant(1.f);
	IntVariable handle = getInVal<IntVariable>("Handle");
	ColorVariable mainColor = getInVal<ColorVariable>("Main Color");
	ColorVariable maskColor = getInVal<ColorVariable>("Mask Color");
	ColorVariable tertColor = getInVal<ColorVariable>("Tertiary Color");
	FloatVariable blend = getInVal<FloatVariable>("Blend");
	FloatVariable premul = getInVal<FloatVariable>("Premul");
	ColorVariable tint = getInVal<ColorVariable>("Tint");
	FloatVariable hue = getInVal<FloatVariable>("Hue");
	FloatVariable saturation = getInVal<FloatVariable>("Saturation");
	FloatVariable lightness = getInVal<FloatVariable>("Lightness");
	TransformResult xfrm = getInVal<TransformResult>("Transform");
	int packedOpts = opts;
	proto.AddDataVariable(handle);
	proto.AddDataVariable(mainColor);
	proto.AddDataVariable(maskColor);
	proto.AddDataVariable(tertColor);
	proto.AddDataVariable(blend);
	proto.AddDataVariable(premul);
	proto.AddDataVariable(tint);
	proto.AddDataVariable(hue);
	proto.AddDataVariable(saturation);
	proto.AddDataVariable(lightness);
	proto.renderJobs.emplace_back(layer, [handle, mainColor, maskColor, tertColor, blend, premul, tint, hue, saturation, lightness, xfrm, packedOpts](RuiExportPrototype& proto) {
		StyleDescriptorOffsets style{};
		FillStyleHdr(style, 5, proto, mainColor, maskColor, tertColor, blend, premul, tint, hue, saturation, lightness);
		uint8_t styleId = (uint8_t)proto.AddStyle(style);
		NestedWidgetOffsets rend{};
		rend.xfrmIdx = proto.transformIndices[xfrm.hash];
		rend.visOff = proto.GetFloatConstantOffset(0.f);
		rend.uiHandle = proto.GetIntDataVariableOffset(handle);
		rend.opts = (uint16_t)packedOpts;
		rend.styleIdx = styleId;
		proto.AddRenderJobData((uint8_t*)&rend, sizeof(rend));
	});
}

std::vector<std::shared_ptr<ImFlow::PinProto>> NestedRenderNode::GetPinInfo() {
	std::vector<std::shared_ptr<ImFlow::PinProto>> info;
	info.push_back(std::make_shared<ImFlow::InPinProto<IntVariable>>("Handle", ImFlow::ConnectionFilter::SameType(), IntVariable(0)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Main Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Mask Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tertiary Color", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Blend", ImFlow::ConnectionFilter::SameType(), FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Premul", ImFlow::ConnectionFilter::SameType(), FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<ColorVariable>>("Tint", ImFlow::ConnectionFilter::SameType(), ColorVariable(1.f, 1.f, 1.f, 1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Hue", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Saturation", isPinNumeric, FloatVariable(1.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<FloatVariable>>("Lightness", isPinNumeric, FloatVariable(0.f)));
	info.push_back(std::make_shared<ImFlow::InPinProto<TransformResult>>("Transform", ImFlow::ConnectionFilter::SameType(), TransformResult()));
	return info;
}

void AddRenderNodes(NodeEditor& editor) {
	editor.AddNodeType<AssetRenderNode>();
	editor.AddNodeType<AssetCircleRenderNode>();
	editor.AddNodeType<TextStyleNode>();
	editor.AddNodeType<TextSizeNode>();
	editor.AddNodeType<TextRenderNode>();
	editor.AddNodeType<VideoRenderNode>();
	editor.AddNodeType<CameraRenderNode>();
	editor.AddNodeType<NestedRenderNode>();
}