
#include "ImageAtlas.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <Windows.h>

#include "ThirdParty/rapidjson/document.h"
#include "Thirdparty/DDSTextureLoader11.h"

#define BCDEC_IMPLEMENTATION
#include "ThirdParty/bcdec.h"


#undef GetObject
namespace fs = std::filesystem;



std::map<uint32_t, Asset_t> imageAssetMap{};
std::vector<ImageAtlas> imageAtlases{};
std::mutex atlasMutex{};

// Hash-to-name lookup loaded from Assets/image_names.txt
static std::unordered_map<uint32_t, std::string> s_imageNameLookup;
// Pak asset GUID -> image path (uiia names are often null in the shipped ui.rpak).
static std::unordered_map<uint64_t, std::string> s_uiiaGuidToName;

// Forward: defined below; used when building guid map from image_names.
unsigned __int64 calculateRpakHash(const char* a1a);
uint32_t calculateUimgHash(const char* a1);

void loadImageNameLookup() {
	s_imageNameLookup.clear();
	s_uiiaGuidToName.clear();
	std::ifstream file(".\\Assets\\image_names.txt");
	if (!file.is_open()) return;
	std::string line;
	while (std::getline(file, line)) {
		// Format: 0xHASH=path/name  (HASH = uimg hash of bare path)
		size_t eq = line.find('=');
		if (eq == std::string::npos) continue;
		uint32_t hash = (uint32_t)strtoul(line.substr(0, eq).c_str(), nullptr, 16);
		std::string path = line.substr(eq + 1);
		if (path.empty()) continue;
		s_imageNameLookup[hash] = path;
		// uiia GUID is StringToGuid of the ui_image/ container path.
		const std::string full = "ui_image/" + path + ".rpak";
		s_uiiaGuidToName[calculateRpakHash(full.c_str())] = path;
		const std::string fullNoExt = "ui_image/" + path;
		s_uiiaGuidToName[calculateRpakHash(fullNoExt.c_str())] = path;
	}
	printf("[SERE] Loaded %zu image name lookups, %zu uiia guids\n",
		s_imageNameLookup.size(), s_uiiaGuidToName.size());
}

void loadImageAtlases() {
	fs::path folderPath = ".\\Assets\\Atlases";

	for (const auto& dirEntry : fs::recursive_directory_iterator(folderPath)) {
		if(!dirEntry.is_regular_file())continue;
		fs::path jsonName = dirEntry;
		if(jsonName.extension()!=".json")continue;

		atlasMutex.lock();
		size_t atlasIndex = imageAtlases.size();
		imageAtlases.emplace_back(jsonName,atlasIndex);
		atlasMutex.unlock();
	}

}

void loadImageAtlasFromRpak(UIImageAtlasAssetHeader_v10_t* hdr, ShaderSizeData_t* shaderData, size_t textureId) {
	atlasMutex.lock();
	size_t atlasIndex = imageAtlases.size();
	imageAtlases.emplace_back(hdr,shaderData,atlasIndex,textureId);
	atlasMutex.unlock();
}

unsigned __int64 calculateRpakHash(const char* a1a) {

	uint32_t* v1; // r8
	uint64_t         v2; // r10
	int                   v3; // er11
	uint32_t         v4; // er9
	uint32_t          i; // edx
	uint64_t         v6; // rcx
	int                   v7; // er9
	int                   v8; // edx
	int                   v9; // eax
	uint32_t        v10; // er8
	int                  v12; // ecx
	uint32_t* a1 = (uint32_t*)a1a;

	v1 = a1;
	v2 = 0i64;
	v3 = 0;
	v4 = (*a1 - 45 * ((~(*a1 ^ 0x5C5C5C5Cu) >> 7) & (((*a1 ^ 0x5C5C5C5Cu) - 0x1010101) >> 7) & 0x1010101)) & 0xDFDFDFDF;
	for (i = ~*a1 & (*a1 - 0x1010101) & 0x80808080; !i; i = v8 & 0x80808080)
	{
		v6 = v4;
		v7 = v1[1];
		++v1;
		v3 += 4;
		v2 = ((((uint64_t)(0xFB8C4D96501i64 * v6) >> 24) + 0x633D5F1 * v2) >> 61) ^ (((uint64_t)(0xFB8C4D96501i64 * v6) >> 24)
			+ 0x633D5F1 * v2);
		v8 = ~v7 & (v7 - 0x1010101);
		v12 = 45 * ((~(v7 ^ 0x5C5C5C5Cu) >> 7) & (((v7 ^ 0x5C5C5C5Cu) - 0x1010101) >> 7) & 0x1010101);
		v4 = (v7 - v12) & 0xDFDFDFDF;

	}
	v9 = -1;
	v10 = (i & -(signed)i) - 1;
	if (_BitScanReverse((unsigned long*)&v12, v10))
	{
		v9 = v12;
	}
	return 0x633D5F1 * v2 + ((0xFB8C4D96501i64 * (uint64_t)(v4 & v10)) >> 24) - 0xAE502812AA7333i64 * (uint32_t)(v3 + v9 / 8);

}



uint32_t calculateUimgHash(const char* a1) {
	uint64_t hash = calculateRpakHash(a1);
	return (uint32_t)hash ^ (hash >> 32);
}

uint32_t loadAsset(const char* a2)
{

	uint32_t nameHash;

	if (!a2 || !*a2)
		return INVALID_ASSET;

	nameHash = calculateUimgHash(a2);

	if (imageAssetMap.contains(nameHash))
		return nameHash;
	nameHash = calculateUimgHash("missing");
	if (imageAssetMap.contains(nameHash))
		return nameHash;
	return INVALID_ASSET;

}




ImageAtlas::ImageAtlas(fs::path& jsonName, size_t atlasIndex):
	shaderDataId(~0LL),
	textureId(~0LL)
{


	std::ifstream jsonFile{jsonName};
	if(jsonFile.fail())return;
	std::stringstream jsonStringStream;
	while (jsonFile.peek() != EOF)
		jsonStringStream << (char)jsonFile.get();
	jsonFile.close();
	rapidjson::Document doc;
	doc.Parse(jsonStringStream.str().c_str());
	rapidjson::ParseErrorCode error = doc.GetParseError();
	if(doc.HasParseError())return;

	rapidjson::GenericObject root = doc.GetObject();

	if((!root.HasMember("textureOffsets")&&root["textureOffsets"].IsArray()))return;
	if((!root.HasMember("textureDimentions")&&root["textureDimentions"].IsArray()))return;
	if((!root.HasMember("textureHashes")&&root["textureHashes"].IsArray()))return;
	if((!root.HasMember("renderOffsets")&&root["renderOffsets"].IsArray()))return;
	if((!root.HasMember("shaderData")&&root["shaderData"].IsArray()))return;




	name = jsonName.filename().replace_extension("").string();
	rapidjson::GenericArray textureOffsets = root["textureOffsets"].GetArray();
	for (auto itr = textureOffsets.Begin(); itr != textureOffsets.End(); itr++) {
		if (!itr->IsObject())continue;
		rapidjson::GenericObject texOff = itr->GetObject();

		if (!(texOff.HasMember("f0") && texOff["f0"].IsNumber()))continue;
		if (!(texOff.HasMember("f1") && texOff["f1"].IsNumber()))continue;
		if (!(texOff.HasMember("endX") && texOff["endX"].IsNumber()))continue;
		if (!(texOff.HasMember("endY") && texOff["endY"].IsNumber()))continue;
		if (!(texOff.HasMember("startX") && texOff["startX"].IsNumber()))continue;
		if (!(texOff.HasMember("startY") && texOff["startY"].IsNumber()))continue;
		if (!(texOff.HasMember("unkX") && texOff["unkX"].IsNumber()))continue;
		if (!(texOff.HasMember("unkY") && texOff["unkY"].IsNumber()))continue;


		textureOffset offset;
		offset.m128_0.m128_f32[0] = texOff["f0"].GetFloat();
		offset.m128_0.m128_f32[1] = texOff["f1"].GetFloat();
		offset.m128_0.m128_f32[2] = texOff["endX"].GetFloat();
		offset.m128_0.m128_f32[3] = texOff["endY"].GetFloat();
		offset.m128_10.m128_f32[0] = texOff["startX"].GetFloat();
		offset.m128_10.m128_f32[1] = texOff["startY"].GetFloat();
		offset.m128_10.m128_f32[2] = texOff["unkX"].GetFloat();
		offset.m128_10.m128_f32[3] = texOff["unkY"].GetFloat();
		offsets.push_back(offset);
	}

	rapidjson::GenericArray textureDimentions = root["textureDimentions"].GetArray();
	for (auto itr = textureDimentions.Begin(); itr != textureDimentions.End(); itr++) {
		if (!itr->IsObject())continue;
		rapidjson::GenericObject dimObj = itr->GetObject();

		if (!(dimObj.HasMember("width") && dimObj["width"].IsInt()))continue;
		if (!(dimObj.HasMember("height") && dimObj["height"].IsInt()))continue;

		ImageAtlasTextureDimention dim;
		dim.width = dimObj["width"].GetInt();
		dim.height = dimObj["height"].GetInt();
		dimentions.push_back(dim);
	}
	rapidjson::GenericArray textureHashes = root["textureHashes"].GetArray();
	for (auto itr = textureHashes.Begin(); itr != textureHashes.End(); itr++) {
		if (!itr->IsObject())continue;
		rapidjson::GenericObject hashObj = itr->GetObject();

		if (!(hashObj.HasMember("hash") && hashObj["hash"].IsUint()))continue;
		if (!(hashObj.HasMember("flags") && hashObj["flags"].IsUint()))continue;

		uint32_t hash = hashObj["hash"].GetUint();
		uint16_t flags = hashObj["flags"].GetUint();
		std::string name;
		if (hashObj.HasMember("name") && hashObj["name"].IsString())
			name = hashObj["name"].GetString();
		else
			name = std::format("0x{:X}", hash);
		imageAssetMap.insert({ hash, {name,atlasIndex,hashes.size(),flags} });
		hashes.push_back(hash);
		names.push_back(name);

	}

	rapidjson::GenericArray renderOffset = root["renderOffsets"].GetArray();
	for (auto itr = renderOffset.Begin(); itr != renderOffset.End(); itr++) {
		if (!itr->IsObject())continue;
		rapidjson::GenericObject renderOff = itr->GetObject();

		if (!(renderOff.HasMember("f0") && renderOff["f0"].IsNumber()))continue;
		if (!(renderOff.HasMember("f1") && renderOff["f1"].IsNumber()))continue;
		if (!(renderOff.HasMember("endX") && renderOff["endX"].IsNumber()))continue;
		if (!(renderOff.HasMember("endY") && renderOff["endY"].IsNumber()))continue;
		if (!(renderOff.HasMember("startX") && renderOff["startX"].IsNumber()))continue;
		if (!(renderOff.HasMember("startY") && renderOff["startY"].IsNumber()))continue;
		if (!(renderOff.HasMember("unkX") && renderOff["unkX"].IsNumber()))continue;
		if (!(renderOff.HasMember("unkY") && renderOff["unkY"].IsNumber()))continue;


		uiImageAtlasUnk offset;
		offset.m128_0.m128_f32[0] = renderOff["f0"].GetFloat();
		offset.m128_0.m128_f32[1] = renderOff["f1"].GetFloat();
		offset.m128_0.m128_f32[2] = renderOff["endX"].GetFloat();
		offset.m128_0.m128_f32[3] = renderOff["endY"].GetFloat();
		offset.m128_10.m128_f32[0] = renderOff["startX"].GetFloat();
		offset.m128_10.m128_f32[1] = renderOff["startY"].GetFloat();
		offset.m128_10.m128_f32[2] = renderOff["unkX"].GetFloat();
		offset.m128_10.m128_f32[3] = renderOff["unkY"].GetFloat();

		renderOffsets.push_back(offset);
	}

	rapidjson::GenericArray shaderDat = root["shaderData"].GetArray();
	for (auto itr = shaderDat.Begin(); itr != shaderDat.End(); itr++) {
		if (!itr->IsObject())continue;
		rapidjson::GenericObject shaderDat = itr->GetObject();

		if (!(shaderDat.HasMember("minX") && shaderDat["minX"].IsNumber()))continue;
		if (!(shaderDat.HasMember("minY") && shaderDat["minY"].IsNumber()))continue;
		if (!(shaderDat.HasMember("maxX") && shaderDat["maxX"].IsNumber()))continue;
		if (!(shaderDat.HasMember("maxY") && shaderDat["maxY"].IsNumber()))continue;

		ShaderSizeData_t shdDat;
		shdDat.minX = shaderDat["minX"].GetFloat();
		shdDat.minY = shaderDat["minY"].GetFloat();
		shdDat.sizeX = shaderDat["maxX"].GetFloat();
		shdDat.sizeY = shaderDat["maxY"].GetFloat();
		shaderData.push_back(shdDat);
	}

	fs::path ddsName = jsonName.replace_extension("dds");

	textureId = g_renderFramework->LoadTexture(ddsName);
	shaderDataId = g_renderFramework->CreateShaderDataBuffer(shaderData);;

}

ImageAtlas::ImageAtlas(UIImageAtlasAssetHeader_v10_t* hdr, ShaderSizeData_t* rawSharderData, size_t atlasIndex,size_t textureID):
	textureId(textureID),
	shaderDataId(0)

{

	offsets.resize(hdr->textureCount);
	memcpy(offsets.data(),hdr->textureOffsets,sizeof(textureOffset)*hdr->textureCount);
	dimentions.resize(hdr->textureCount);
	memcpy(dimentions.data(),hdr->textureDimensions,sizeof(ImageAtlasTextureDimention)*hdr->textureCount);
	shaderData.resize(hdr->textureCount);
	memcpy(shaderData.data(),rawSharderData,sizeof(ShaderSizeData_t)*hdr->textureCount);

	renderOffsets.resize(hdr->renderOffsetCount);
	memcpy(renderOffsets.data(),hdr->renderOffsets,sizeof(textureOffset)*hdr->renderOffsetCount);
	for (uint16_t i = 0; i < hdr->textureCount; i++) {

		UiAtlasImageHash& img = hdr->textureHashes[i];
		
		std::string name;
		if(hdr->textureNames)
			name = &hdr->textureNames[img.nameOffset];
		else if(auto it = s_imageNameLookup.find(img.hash); it != s_imageNameLookup.end())
			name = it->second;
		else
			name = std::format("0x{:X}",img.hash);
		imageAssetMap.insert({ img.hash, {name,atlasIndex,hashes.size(),img.flags} });
		hashes.push_back(img.hash);
	}
	shaderDataId = g_renderFramework->CreateShaderDataBuffer(shaderData);

}




// Transparent BC1 4x4 block (8 B) — all alpha 0.
static void FillBc1ClearBlock(uint8_t* b)
{
	b[0] = 0x00; b[1] = 0x00; b[2] = 0xFF; b[3] = 0xFF;
	b[4] = 0xFF; b[5] = 0xFF; b[6] = 0xFF; b[7] = 0xFF;
}

static void RegisterUiiaNameHashes(const std::string& name, size_t atlasIndex)
{
	auto add = [&](const std::string& n) {
		if (n.empty()) return;
		const uint32_t h = calculateUimgHash(n.c_str());
		imageAssetMap[h] = { n, atlasIndex, 0, 0 };
	};
	add(name);
	std::string bare = name;
	if (bare.rfind("ui_image/", 0) == 0)
		bare = bare.substr(9);
	if (bare.size() > 5) {
		const std::string suf = bare.substr(bare.size() - 5);
		if (suf == ".rpak")
			bare = bare.substr(0, bare.size() - 5);
	}
	add(bare);
	add("ui_image/" + bare);
	add("ui_image/" + bare + ".rpak");
}

static size_t ReadableBytesFrom(const void* p)
{
	if (!p) return 0;
	MEMORY_BASIC_INFORMATION mbi{};
	if (!VirtualQuery(p, &mbi, sizeof(mbi)))
		return 0;
	if (mbi.State != MEM_COMMIT)
		return 0;
	if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
		return 0;
	const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
	const uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	if (addr < base) return 0;
	return mbi.RegionSize - (addr - base);
}

// On-disk 4x4 block slots inside a 32x32 tile are Morton-ordered (engine + RSX).
// linear_block(bx, by) <- ondisk[Morton2D(bx, by)]. Identical for BC1 and BC7.
static uint32_t UiiaMorton2D(uint32_t x, uint32_t y)
{
	uint32_t m = 0;
	for (int b = 0; b < 3; b++) {
		m |= ((x >> b) & 1u) << (2 * b);
		m |= ((y >> b) & 1u) << (2 * b + 1);
	}
	return m;
}

static void UiiaInitDemorton(uint8_t demX[64], uint8_t demY[64])
{
	for (uint32_t by = 0; by < 8; by++) {
		for (uint32_t bx = 0; bx < 8; bx++) {
			const uint32_t s = UiiaMorton2D(bx, by);
			demX[s] = (uint8_t)bx;
			demY[s] = (uint8_t)by;
		}
	}
}

// Demorton on-disk tiles into a linear BC block plane (row-major 4x4 blocks).
static bool BuildUiiaBcPlane(const char* srcBase, size_t readable,
	const std::vector<UIImageTile_t>& tiles, uint32_t widthBlocks, uint32_t heightBlocks,
	uint32_t wantOp, uint32_t blockBytes, uint32_t tileBytes,
	const uint8_t demX[64], const uint8_t demY[64],
	std::vector<uint8_t>& outPlane)
{
	static constexpr uint32_t BPT = 8u;
	const uint32_t sheetBlocksX = widthBlocks * BPT;
	const uint32_t sheetBlocksY = heightBlocks * BPT;
	const size_t rowStride = (size_t)sheetBlocksX * blockBytes;
	const size_t planeSize = rowStride * sheetBlocksY;
	if (planeSize == 0 || planeSize > 64ull * 1024ull * 1024ull)
		return false;

	outPlane.assign(planeSize, 0);
	if (wantOp == 0x40u) {
		for (size_t i = 0; i + 8 <= planeSize; i += 8)
			FillBc1ClearBlock(outPlane.data() + i);
	}

	for (uint32_t ty = 0; ty < heightBlocks; ty++) {
		for (uint32_t tx = 0; tx < widthBlocks; tx++) {
			const UIImageTile_t& tile = tiles[tx + ty * widthBlocks];
			if (tile.opcode != wantOp)
				continue;
			const size_t srcOff = tile.offset;
			if (srcOff + tileBytes > readable)
				continue;
			const char* tileSrc = srcBase + srcOff;
			for (uint32_t slot = 0; slot < 64; slot++) {
				const uint32_t bx = demX[slot];
				const uint32_t by = demY[slot];
				const uint32_t gx = tx * BPT + bx;
				const uint32_t gy = ty * BPT + by;
				const size_t dst = (size_t)gy * rowStride + (size_t)gx * blockBytes;
				if (dst + blockBytes > planeSize)
					continue;
				memcpy(outPlane.data() + dst, tileSrc + (size_t)slot * blockBytes, blockBytes);
			}
		}
	}
	return true;
}

static void DecodeBcPlaneToRgba(const uint8_t* plane, bool isBc7,
	uint32_t sheetW, uint32_t sheetH, std::vector<uint8_t>& rgba)
{
	const uint32_t blockBytes = isBc7 ? 16u : 8u;
	const uint32_t blocksX = sheetW / 4u;
	const uint32_t blocksY = sheetH / 4u;
	const int pitch = (int)(sheetW * 4u);
	rgba.assign((size_t)sheetW * sheetH * 4u, 0);
	const size_t rowStride = (size_t)blocksX * blockBytes;

	for (uint32_t by = 0; by < blocksY; by++) {
		for (uint32_t bx = 0; bx < blocksX; bx++) {
			const uint8_t* blk = plane + (size_t)by * rowStride + (size_t)bx * blockBytes;
			uint8_t* dst = rgba.data() + ((size_t)by * 4u * (size_t)pitch) + ((size_t)bx * 4u * 4u);
			if (isBc7)
				bcdec_bc7(blk, dst, pitch);
			else
				bcdec_bc1(blk, dst, pitch);
		}
	}
}

// Full RSX CreateTextureForImage path:
// demorton BC planes -> RGBA -> composite BC7 over BC1 -> 31px stride crop to logical WxH.
// routeBits: LQ flags&3, HQ (flags>>2)&3 — 1 BC1, 2 BC7, 3 mixed table.
static size_t DecodeUiiaPlane(const char* srcBase, size_t readable, uint16_t imgW, uint16_t imgH,
	uint16_t flagsRaw, uint16_t routeBits,
	uint16_t& outW, uint16_t& outH, uint16_t& outShiftedW, uint16_t& outShiftedH)
{
	static constexpr uint32_t BC1_TILE = 512u;
	static constexpr uint32_t BC7_TILE = 1024u;
	static constexpr uint32_t BC1_OP = 0x40u;
	static constexpr uint32_t BC7_OP = 0x41u;
	static constexpr uint32_t MAX_BLOCKS = 256u;
	static constexpr uint32_t TILE_PX = 32u;
	static constexpr uint32_t TILE_STRIDE = 31u;

	static uint8_t demX[64], demY[64];
	static bool demInit = false;
	if (!demInit) {
		UiiaInitDemorton(demX, demY);
		demInit = true;
	}

	outW = imgW;
	outH = imgH;
	outShiftedW = 0;
	outShiftedH = 0;

	if (!srcBase || imgW == 0 || imgH == 0 || readable < 16)
		return ~0ull;

	const uint16_t unkWOff = (~(flagsRaw >> 5) & 2);
	const uint16_t unkHOff = (~(flagsRaw >> 6) & 2);
	constexpr uint32_t pixelOffset = 29;
	constexpr float rcpBlock = 1.f / 31.f;

	uint32_t widthBlocks = (uint32_t)((imgW + unkWOff + pixelOffset) * rcpBlock);
	uint32_t heightBlocks = (uint32_t)((imgH + unkHOff + pixelOffset) * rcpBlock);
	if (!widthBlocks) widthBlocks = 1;
	if (!heightBlocks) heightBlocks = 1;
	if (widthBlocks > MAX_BLOCKS || heightBlocks > MAX_BLOCKS)
		return ~0ull;

	const uint32_t totalTiles = widthBlocks * heightBlocks;
	if (totalTiles == 0 || totalTiles > MAX_BLOCKS * MAX_BLOCKS)
		return ~0ull;

	const uint16_t route = (uint16_t)(routeBits & 3u);
	const bool hasTable = (route == 3u);
	const bool pureBc7 = (route == 2u);

	std::vector<UIImageTile_t> tiles(totalTiles);

	if (hasTable) {
		const size_t tableBytes = (size_t)totalTiles * sizeof(UIImageTile_t);
		if (tableBytes > readable)
			return ~0ull;
		memcpy(tiles.data(), srcBase, tableBytes);
		for (uint32_t i = 0; i < totalTiles; i++) {
			if (tiles[i].opcode == 0xC0) {
				if (tiles[i].offset >= totalTiles)
					return ~0ull;
				tiles[i] = tiles[tiles[i].offset];
			}
		}
	} else {
		const uint32_t tileSize = pureBc7 ? BC7_TILE : BC1_TILE;
		const uint32_t wantOp = pureBc7 ? BC7_OP : BC1_OP;
		const size_t need = (size_t)totalTiles * tileSize;
		if (need > readable)
			return ~0ull;
		for (uint32_t i = 0; i < totalTiles; i++) {
			tiles[i].opcode = wantOp;
			tiles[i].offset = i * tileSize;
		}
	}

	uint32_t numBc1 = 0, numBc7 = 0;
	for (uint32_t i = 0; i < totalTiles; i++) {
		if (tiles[i].opcode == BC1_OP) numBc1++;
		else if (tiles[i].opcode == BC7_OP) numBc7++;
	}
	if (numBc1 == 0 && numBc7 == 0)
		return ~0ull;

	const uint32_t sheetW = widthBlocks * TILE_PX;
	const uint32_t sheetH = heightBlocks * TILE_PX;
	const size_t sheetBytes = (size_t)sheetW * sheetH * 4u;
	if (sheetBytes == 0 || sheetBytes > 256ull * 1024ull * 1024ull)
		return ~0ull;

	std::vector<uint8_t> sheetRgba(sheetBytes, 0);

	// BC1 base (transparent clear for missing tiles).
	if (numBc1 > 0) {
		std::vector<uint8_t> bc1Plane;
		if (!BuildUiiaBcPlane(srcBase, readable, tiles, widthBlocks, heightBlocks,
			BC1_OP, 8u, BC1_TILE, demX, demY, bc1Plane))
			return ~0ull;
		std::vector<uint8_t> bc1Rgba;
		DecodeBcPlaneToRgba(bc1Plane.data(), false, sheetW, sheetH, bc1Rgba);
		sheetRgba.swap(bc1Rgba);
	}

	// BC7 plane composited over BC1 (RSX: CopySourceTextureSlice per BC7 tile).
	if (numBc7 > 0) {
		std::vector<uint8_t> bc7Plane;
		if (!BuildUiiaBcPlane(srcBase, readable, tiles, widthBlocks, heightBlocks,
			BC7_OP, 16u, BC7_TILE, demX, demY, bc7Plane))
			return ~0ull;
		std::vector<uint8_t> bc7Rgba;
		DecodeBcPlaneToRgba(bc7Plane.data(), true, sheetW, sheetH, bc7Rgba);

		if (numBc1 == 0) {
			sheetRgba.swap(bc7Rgba);
		} else {
			for (uint32_t ty = 0; ty < heightBlocks; ty++) {
				for (uint32_t tx = 0; tx < widthBlocks; tx++) {
					if (tiles[tx + ty * widthBlocks].opcode != BC7_OP)
						continue;
					for (uint32_t py = 0; py < TILE_PX; py++) {
						const size_t srcRow = ((size_t)(ty * TILE_PX + py) * sheetW + (size_t)tx * TILE_PX) * 4u;
						memcpy(sheetRgba.data() + srcRow, bc7Rgba.data() + srcRow, TILE_PX * 4u);
					}
				}
			}
		}
	}

	// 31-pixel stride crop into logical image (tiles are 32px, advance by 31).
	std::vector<uint8_t> finalRgba((size_t)imgW * imgH * 4u, 0);
	for (uint32_t ty = 0; ty < heightBlocks; ty++) {
		const uint32_t sizeY = (ty * TILE_STRIDE + 30u < imgH) ? TILE_STRIDE : (imgH - ty * TILE_STRIDE);
		if (sizeY == 0 || sizeY > TILE_PX)
			continue;
		for (uint32_t tx = 0; tx < widthBlocks; tx++) {
			const uint32_t sizeX = (tx * TILE_STRIDE + 30u < imgW) ? TILE_STRIDE : (imgW - tx * TILE_STRIDE);
			if (sizeX == 0 || sizeX > TILE_PX)
				continue;
			for (uint32_t py = 0; py < sizeY; py++) {
				const size_t src = ((size_t)(ty * TILE_PX + py) * sheetW + (size_t)tx * TILE_PX) * 4u;
				const size_t dst = ((size_t)(ty * TILE_STRIDE + py) * imgW + (size_t)tx * TILE_STRIDE) * 4u;
				memcpy(finalRgba.data() + dst, sheetRgba.data() + src, (size_t)sizeX * 4u);
			}
		}
	}

	// Texture is exact logical size — full UV, no sheet crop.
	outW = imgW;
	outH = imgH;
	outShiftedW = imgW;
	outShiftedH = imgH;

	const uint32_t pitch = (uint32_t)imgW * 4u;
	const uint32_t slicePitch = pitch * imgH;
	// format index 31 = R8G8B8A8_UNORM in s_PakToDxgiFormat
	return g_renderFramework->CreateTextureFromData(
		finalRgba.data(), imgW, imgH, 31, pitch, slicePitch);
}

void loadImageFromUiia(UIImageAssetHeader_v2_t* hdr, UIImageAssetData_v2_t* cpu, uint64_t guid,
	const void* hqData, size_t hqSize)
{
	if (!hdr || !cpu)
		return;

	// Name: PagePtr after ResolvePointers. Often null -- resolve via guid map.
	std::string name;
	if (cpu->name && reinterpret_cast<uintptr_t>(cpu->name) > 0x10000) {
		name = cpu->name;
	}
	if (name.empty() && guid) {
		if (auto it = s_uiiaGuidToName.find(guid); it != s_uiiaGuidToName.end())
			name = it->second;
	}
	if (name.empty() && guid)
		name = std::format("uiia_{:x}", guid);
	if (name.empty())
		name = std::format("uiia_{:x}", (uintptr_t)hdr);

	// Strip ui_image/ + .rpak for bare path hashing when present.
	if (name.rfind("ui_image/", 0) == 0) {
		name = name.substr(9);
		if (name.size() > 5 && name.substr(name.size() - 5) == ".rpak")
			name = name.substr(0, name.size() - 5);
	}

	const uint16_t flagsRaw = *reinterpret_cast<const uint16_t*>(&hdr->imgFlags);
	const uint16_t lqW = cpu->lowResWidth ? cpu->lowResWidth : (hdr->width ? hdr->width : 1);
	const uint16_t lqH = cpu->lowResHeight ? cpu->lowResHeight : (hdr->height ? hdr->height : 1);
	const uint16_t hqW = cpu->highResWidth ? cpu->highResWidth : 0;
	const uint16_t hqH = cpu->highResHeight ? cpu->highResHeight : 0;

	uint16_t decW = 1, decH = 1, shiftedW = 0, shiftedH = 0;
	size_t textureId = ~0ull;
	const char* qualityTag = "lq";

	// Prefer starpak HQ when present and larger than permanent LQ.
	if (hqData && hqSize >= 16 && hqW > 0 && hqH > 0 && (hqW > lqW || hqH > lqH || hqW * hqH > lqW * lqH)) {
		try {
			// HQ route bits live in hasHighTableBc1/Bc7 (flags bits 2-3).
			textureId = DecodeUiiaPlane(static_cast<const char*>(hqData), hqSize, hqW, hqH,
				flagsRaw, (uint16_t)((flagsRaw >> 2) & 3u), decW, decH, shiftedW, shiftedH);
			if (textureId != ~0ull)
				qualityTag = "hq";
		} catch (...) {
			textureId = ~0ull;
		}
	}

	// Permanent LQ (always available while pak pages live).
	if (textureId == ~0ull && cpu->data) {
		try {
			const size_t readable = ReadableBytesFrom(cpu->data);
			textureId = DecodeUiiaPlane(static_cast<const char*>(cpu->data), readable, lqW, lqH,
				flagsRaw, (uint16_t)(flagsRaw & 3u), decW, decH, shiftedW, shiftedH);
			qualityTag = "lq";
		} catch (...) {
			textureId = ~0ull;
		}
	}

	if (textureId == ~0ull) {
		// Fall back: shared white so boot still completes.
		static size_t s_shared = ~0ull;
		if (s_shared == ~0ull) {
			uint32_t pixel = 0xFFFFFFFFu;
			s_shared = g_renderFramework->CreateTextureFromData(&pixel, 1, 1, 31, 4, 4);
		}
		textureId = s_shared;
		if (textureId == ~0ull)
			return;
		decW = hdr->width ? hdr->width : 1;
		decH = hdr->height ? hdr->height : 1;
		shiftedW = decW;
		shiftedH = decH;
		qualityTag = "white";
	}

	const uint16_t w = hdr->width ? hdr->width : decW;
	const uint16_t h = hdr->height ? hdr->height : decH;
	// Texture is already 31-stride cropped to logical size — full UV.
	const float uvX = 1.f;
	const float uvY = 1.f;
	(void)shiftedW;
	(void)shiftedH;

	atlasMutex.lock();
	const size_t atlasIndex = imageAtlases.size();

	UIImageAtlasAssetHeader_v10_t fake{};
	textureOffset off{};
	float* fo = reinterpret_cast<float*>(&off);
	fo[0] = hdr->unknownFloats[0];
	fo[1] = hdr->unknownFloats[1];
	fo[2] = hdr->unknownFloats[2] != 0.f ? hdr->unknownFloats[2] : 1.f;
	fo[3] = hdr->unknownFloats[3] != 0.f ? hdr->unknownFloats[3] : 1.f;
	fo[4] = hdr->unknownFloats[4];
	fo[5] = hdr->unknownFloats[5];
	fo[6] = hdr->unknownFloats[6] != 0.f ? hdr->unknownFloats[6] : 1.f;
	fo[7] = hdr->unknownFloats[7] != 0.f ? hdr->unknownFloats[7] : 1.f;

	ImageAtlasTextureDimention dim{ w, h };
	const uint32_t primaryHash = calculateUimgHash(name.c_str());
	UiAtlasImageHash hashEnt{};
	hashEnt.hash = primaryHash;
	hashEnt.flags = 0;
	hashEnt.nameOffset = 0;
	ShaderSizeData_t shd{ 0.f, 0.f, uvX, uvY };
	std::string nameStorage = name;
	fake.widthRatio = w ? (1.f / (float)w) : 1.f;
	fake.heightRatio = h ? (1.f / (float)h) : 1.f;
	fake.width = w;
	fake.height = h;
	fake.textureCount = 1;
	fake.renderOffsetCount = 0;
	fake.textureOffsets = &off;
	fake.textureDimensions = &dim;
	fake.renderOffsets = nullptr;
	fake.textureHashes = &hashEnt;
	fake.textureNames = nameStorage.c_str();
	fake.atlasGUID = 0;

	imageAtlases.emplace_back(&fake, &shd, atlasIndex, textureId);
	RegisterUiiaNameHashes(name, atlasIndex);
	imageAssetMap[primaryHash] = { name, atlasIndex, 0, 0 };
	atlasMutex.unlock();

	static int s_uiiaLogLeft = 24;
	static int s_hqOk = 0, s_lqOk = 0;
	if (qualityTag[0] == 'h') s_hqOk++;
	else if (qualityTag[0] == 'l') s_lqOk++;
	if (s_uiiaLogLeft-- > 0)
		printf("[SERE] UIIA '%s' hash=0x%X %ux%u (%s %ux%u sheet %ux%u) atlas=%zu\n",
			name.c_str(), primaryHash, w, h, qualityTag, decW, decH, shiftedW, shiftedH, atlasIndex);
	// Final tallies once every so often after the sample window.
	if (s_uiiaLogLeft == 0)
		printf("[SERE] UIIA quality so far: hq=%d lq=%d (sample of first loads)\n", s_hqOk, s_lqOk);
}

void clearImageAtlases() {
	atlasMutex.lock();
	imageAssetMap.clear();
	imageAtlases.clear();
	atlasMutex.unlock();
}