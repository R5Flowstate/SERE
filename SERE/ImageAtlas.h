#pragma once

#include <intrin.h>
#include <map>
#include <vector>
#include <string>
#include "RenderFrameworks/RenderFramework.h"
#include <filesystem>

#define INVALID_ASSET 0xFFFFFFFF

namespace fs = std::filesystem;

struct textureOffset
{
	__m128 m128_0;
	__m128 m128_10;
};

struct ImageAtlasTextureDimention
{
	uint16_t width;
	uint16_t height;
};

struct uiImageAtlasUnk
{
	__m128 m128_0;
	__m128 m128_10;
};


struct Asset_t {
	std::string name;
	size_t atlasIndex;
	size_t imageIndex;
	uint16_t flags;
};

struct UiAtlasImageHash {
	uint32_t hash;
	uint16_t flags;
	uint16_t nameOffset;
};


struct UIImageAtlasAssetHeader_v10_t
{
	float widthRatio;
	float heightRatio;

	uint16_t width;
	uint16_t height;

	uint16_t textureCount;
	uint16_t renderOffsetCount;

	textureOffset* textureOffsets;
	ImageAtlasTextureDimention* textureDimensions;

	uiImageAtlasUnk* renderOffsets;

	UiAtlasImageHash* textureHashes;
	const char* textureNames;
	uint64_t atlasGUID;
};

// S21 uiia v2 (single image). Layout matches RSX ui_image.h.
#pragma pack(push, 1)
struct UIImageAssetFlags_v2_t
{
	uint8_t hasLowTableBc1 : 1;
	uint8_t hasLowTableBc7 : 1;
	uint8_t hasHighTableBc1 : 1;
	uint8_t hasHighTableBc7 : 1;
	uint8_t compressionType : 2;
	uint8_t sizeShift : 2;
	uint8_t unkBits;
};
static_assert(sizeof(UIImageAssetFlags_v2_t) == 2, "uiia flags");

struct UIImageAssetHeader_v2_t
{
	uint32_t unk_00;
	uint32_t unk_04;
	int32_t unk_08;
	uint32_t streamedOffset;
	uint16_t streamedSize;
	UIImageAssetFlags_v2_t imgFlags;
	uint8_t streamState;
	uint8_t _pad15;
	int16_t borderId;
	uint16_t width;
	uint16_t height;
	uint32_t unk_1C;
	float unknownFloats[8];
};
static_assert(sizeof(UIImageAssetHeader_v2_t) == 0x40, "uiia hdr v2");

struct UIImageAssetData_v2_t
{
	float borderFloats[8];
	uint16_t highResWidth;
	uint16_t highResHeight;
	uint16_t lowResWidth;
	uint16_t lowResHeight;
	void* data;
	char* name;
	char unk_38[8];
};
static_assert(sizeof(UIImageAssetData_v2_t) == 0x40, "uiia cpu v2");

struct UIImageTile_t
{
	uint32_t offset : 24;
	uint32_t opcode : 8;
};
static_assert(sizeof(UIImageTile_t) == 4, "uiia tile");
#pragma pack(pop)



struct ImageAtlas {

	ImageAtlas(fs::path& jsonName, size_t atlasIndex);
	ImageAtlas(UIImageAtlasAssetHeader_v10_t* hdr,ShaderSizeData_t* sharderData, size_t atlasIndex, size_t textureID);
	std::string name;
	std::vector<textureOffset> offsets;
	std::vector<ImageAtlasTextureDimention> dimentions;
	std::vector<uiImageAtlasUnk> renderOffsets;
	std::vector<uint32_t> hashes;
	std::vector<std::string> names;
	size_t textureId;
	size_t shaderDataId;
	std::vector<ShaderSizeData_t> shaderData;

	void* GetImageView() const {
		return g_renderFramework->GetTextureView(textureId);
	};
};


uint32_t loadAsset(const char* a2);
void loadImageAtlases();
void loadImageNameLookup();
void loadImageAtlasFromRpak(UIImageAtlasAssetHeader_v10_t* hdr, ShaderSizeData_t* shaderData, size_t textureId);
// S21: decode uiia tiles into a D3D texture and register path hashes (UIMG-compatible).
// Prefer HQ from starpak when hqData is set; else permanent LQ in cpu->data.
// guid = pak asset GUID — used to resolve null names via image_names.txt reverse map.
void loadImageFromUiia(UIImageAssetHeader_v2_t* hdr, UIImageAssetData_v2_t* cpu, uint64_t guid = 0,
	const void* hqData = nullptr, size_t hqSize = 0);

void clearImageAtlases();

extern std::map<uint32_t,Asset_t> imageAssetMap;
extern std::vector<ImageAtlas> imageAtlases;
