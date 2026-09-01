#pragma once
#include <map>
#include <vector>
#include <set>
#include <any>
#include <memory>
#include <unordered_map>
#include <fstream>

struct RuiExportPrototype;

#include "Imgui/ImNodeFlow.h"
#include "RuiNodeEditor/RuiVariables.h"
#include "RuiNodeEditor/Mapping.h"
#include "RuiRendering/RenderManager.h"
#include "RuiNodeEditor/RuiBaseNode.h"

template<typename T>
struct ExportElement {
#ifdef _DEBUG
	std::string sourceNodeName;
#endif
	T identifier;
	std::set<T> dependencys;
	std::function<void(RuiExportPrototype&)> callback;
};


struct Float2Offsets {
	uint16_t x;
	uint16_t y;
};

struct Float3Offsets {
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct ColorOffsets {
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t alpha;
};

// Style descriptor (68 bytes)
// Field names are from text style perspective; ellipse/asset reuse same offsets
struct StyleDescriptorOffsets {
	uint16_t type = 0;           // 0x00
	ColorOffsets color0;         // 0x02 (8B)
	ColorOffsets color1;         // 0x0A (8B)
	ColorOffsets color2;         // 0x12 (8B)
	uint16_t tint[4] = {};      // 0x1A (8B) - tint RGBA
	uint16_t blend = 0;         // 0x22
	uint16_t premul = 0;        // 0x24
	uint16_t hue = 0;           // 0x26 - HSL
	uint16_t saturation = 0;    // 0x28
	uint16_t lightness = 0;     // 0x2A
	uint16_t fontHash = 0;      // 0x2C - font hash (text) / innerSliceBlend (ellipse)
	uint16_t shadowAlpha = 0;   // 0x2E - shadow hardness (text) / sliceBegin (ellipse)
	uint16_t shadowOffsetX = 0; // 0x30 - (text) / sliceEnd (ellipse)
	uint16_t shadowOffsetY = 0; // 0x32 - (text) / ellipseSize.x (ellipse)
	uint16_t shadowBlur = 0;    // 0x34 - (text) / ellipseSize.y (ellipse)
	uint16_t pixelHeight = 0;   // 0x36 - font size (text) / innerMask (ellipse)
	uint16_t pixelAspect = 0;   // 0x38 - stretchX (text) / vingette (ellipse)
	uint16_t outlineWidth = 0;  // 0x3A - backgroundSize (text)
	uint16_t thicken = 0;       // 0x3C - boldness (text)
	uint16_t blur = 0;          // 0x3E
	uint16_t baselineShift = 0; // 0x40
	uint16_t kerning = 0;       // 0x42
}; // 68 bytes total
static_assert(sizeof(StyleDescriptorOffsets) == 68, "StyleDescriptorOffsets must be 68 bytes");

// RUIP v2 header (172 bytes) = v1 base (160 bytes) + v2 extension (12 bytes)
#pragma pack(push, 1)
struct RuiPackageHeader_v2_t {
	// v1 base (160 bytes)
	uint32_t magic;
	uint16_t packageVersion;
	uint16_t ruiVersion;
	uint64_t nameOffset;
	float elementWidth;
	float elementHeight;
	float elementWidthRcp;
	float elementHeightRcp;
	uint16_t defaultValuesSize;
	uint16_t dataStructSize;
	uint16_t styleDescriptorCount;
	uint16_t unk_A4;
	uint16_t renderJobCount;
	uint16_t argClusterCount;
	uint16_t argCount;
	uint16_t keyframingCount;
	uint16_t transformDataSize;
	uint16_t nameSize;
	uint16_t rpakPointersInDefaltDataCount;
	uint8_t pad[2];
	uint32_t argNamesSize;
	uint32_t renderJobSize;
	uint32_t keyframingSize;
	uint32_t defaultStringsSize;
	uint64_t argNamesOffset;
	uint64_t argClusterOffset;
	uint64_t argumentsOffset;
	uint64_t styleDescriptorOffset;
	uint64_t renderJobOffset;
	uint64_t keyframingOffset;
	uint64_t transformDataOffset;
	uint64_t defaultValuesOffset;
	uint64_t defaultStringDataOffset;
	uint64_t rpakPointersInDefaultDataOffset;
	uint64_t defaultStringsDataSize;
	// v2 extension (12 bytes)
	uint32_t pointerFixupCount;
	uint64_t pointerFixupOffset;
};
#pragma pack(pop)
static_assert(sizeof(RuiPackageHeader_v2_t) == 172, "RuiPackageHeader_v2_t must be 172 bytes");


struct Argument_t
{
	VariableType type;

	uint8_t unk_1;

	uint16_t dataOffset;
	uint16_t nameOffset;

	uint16_t shortHash;

	Argument_t(): type(VariableType::NONE),
		unk_1(0),
		dataOffset(0),
		nameOffset(0),
		shortHash(0)
	{}
};

// UiSymbolTable_s. One per arg cluster; the engine indexes widgets and vars per table,
// so an array/repeat cluster is a second table with maxElemsInArray > 1.
struct ArgCluster_t
{
	uint16_t argIndex;          // varHashBegin
	uint16_t argCount;          // varHashCount

	uint8_t byte_4;             // varHashMagicScale
	uint8_t byte_5;             // varHashMagicBias

	uint16_t short_6;           // literalDataBegin
	uint16_t valueSize;         // literalDataSize
	uint16_t dataStructSize;    // runtimeDataSize
	uint16_t short_C;           // maxElemsInArray
	uint16_t short_E;           // widgetBeginByteOfs
	uint16_t renderJobCount;    // widgetCount
};

struct ExportRenderJob {
	int layer;
	std::function<void(RuiExportPrototype&)> func;
};

struct RuiExportPrototype {



	std::string name;
	Vector2 size;
	std::map<std::string, VariableType> arguments;
	std::map<std::string, VariableType> varsInDataStruct;

	std::vector<ExportElement<std::string>> codeElements;
	std::vector<ExportElement<uint64_t>> transformCallbacks;
	std::vector<ExportRenderJob> renderJobs;
	uint16_t currentDataStructSize = 0;
	uint16_t textCacheIdx = 0;
	std::map<float, uint16_t> floatConstants;
	std::map<int, uint16_t> intConstants;
	std::map<std::string, uint16_t> stringConstants;
	//code export
	std::vector<std::string> codeLines;


	//data struct gen
	std::map<std::string, int> varOffsets;
	std::vector<Mapping> mappings; // used by MappingNode (currently disabled)
	std::map<uint64_t, uint16_t> transformIndices;
	std::vector<StyleDescriptorOffsets> styleDescriptor;

	// Widgets index styles with a uint8, so identical descriptors must share a
	// slot: a graph that repeats one look across many widgets would otherwise
	// truncate past 255 without a word.
	uint16_t AddStyle(const StyleDescriptorOffsets& style) {
		for (size_t i = 0; i < styleDescriptor.size(); i++) {
			if (memcmp(&styleDescriptor[i], &style, sizeof(style)) == 0)
				return (uint16_t)i;
		}
		styleDescriptor.push_back(style);
		return (uint16_t)(styleDescriptor.size() - 1);
	}
	std::vector<uint8_t> transformData;
	std::vector<uint8_t> renderJobData;
	std::vector<uint8_t> defaultValues;
	std::vector<uint16_t> rpakPointersInDefaultValues;
	std::stringstream defaultStrings;
	uint16_t renderJobCount;
	ArgCluster_t cluster{};
	std::vector<Argument_t> exportArgs;

	// v2 pointer fixups (srcSection, srcOffset, dstSection, dstOffset)
	struct PointerFixup_t {
		uint32_t srcSection;
		uint32_t srcOffset;
		uint32_t dstSection;
		uint32_t dstOffset;
	};
	std::vector<PointerFixup_t> pointerFixups;

	RuiExportPrototype(const RenderInstance& inst,const std::string& name);

	void AddConstant(float f);
	void AddConstant(std::string s);
	void AddIntConstant(int v);

	void AddTransformData(uint8_t* data, size_t size);

	void AddRenderJobData(uint8_t* data, size_t size);

	void AddDataVariable(const FloatVariable& var);
	void AddDataVariable(const Float2Variable& var);
	void AddDataVariable(const ColorVariable& var);
	void AddDataVariable(const AssetVariable& var);
	void AddDataVariable(const StringVariable& var);
	void AddDataVariable(const IntVariable& var);

	uint16_t GetFloatDataVariableOffset(const FloatVariable& var);
	Float2Offsets GetFloat2DataVariableOffset(const Float2Variable& var);
	Float3Offsets GetFloat3DataVariableOffset(const Float3Variable& var);
	ColorOffsets GetColorDataVariableOffset(const ColorVariable& var);

	uint16_t GetAssetDataVariableOffset(const AssetVariable& var);
	uint16_t GetStringDataVariableOffset(const StringVariable& var);
	uint16_t GetIntDataVariableOffset(const IntVariable& var);

	uint16_t GetFloatConstantOffset(float f);
	uint16_t GetStringConstantOffset(std::string s);
	uint16_t GetIntConstantOffset(int v);


	void GenerateCode();
	void GenerateTransformData();
	void GenerateRenderJobData();
	void GenerateVariables(std::map<std::string,std::any>& argValues);
	void GenerateArguments();
	bool GenerateCodeStruct();
	void Generate(std::unordered_map<ImFlow::NodeUID, std::shared_ptr<ImFlow::BaseNode>>& nodes, RenderInstance& render);

	void WriteToFile(fs::path path);
	bool hasTypeInCodeStruct(VariableType type) const;

};