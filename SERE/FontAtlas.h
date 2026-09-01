#pragma once


#include <map>
#include <filesystem>
#include "RenderFrameworks/RenderFramework.h"

namespace fs = std::filesystem;




struct KerningInfo_t
{
    int otherChar;
    float kerningDistance;
};


struct Proportion_t
{
    float scaleBounds; // for the addition size added on to of the base size
    float scaleSize; // for the basic character size
};

struct GlyphChunk_t {
    uint16_t glyphIndex;
    uint64_t mask;
};

struct FontShaderData_t {
    float minX;
    float minY;
    float maxX;
    float maxY;
};

struct Glyph_t
{
    float float_0;
    uint16_t kerningStartIndex;
    uint8_t byte_6;
    uint8_t proportionIndex;
    float posBaseX;
    float posBaseY;
    float posMinX;
    float posMinY;
    float posMaxX;
    float posMaxY;
};

struct UIFontHeader_v7_t
{
    char* name;

    uint16_t fontIndex;// index used in engine/by ruis to get this fonts

    uint16_t numProportions; // number of character proportions

    // number of chunks for glyph and unicode lookup, 64 unique glyphs/unicodes per chunk
    uint16_t numKernings; // unused
    uint16_t numUnicodeChunks;

    int glyphIndex; // base glyph index, unused
    int unicodeIndex; // base unicode character index, this gets subtracted off input characters, everything before this index is invalid.

    uint32_t numTextures; // used for mem alloc

    // for scaling the character proportions
    float proportionScaleX;
    float proportionScaleY;

    float unk_24;
    float unk_28;

    // base index for texture count
    uint32_t textureIndex; // used for mem alloc

    // for getting a texture from a provided unicode/glyph
    uint16_t* unicodeChunks; // index per 64 unicodes, index into other arrays to get assigned texture (unicodeBaseIndex & unicodeIndexMask)
    uint16_t* unicodeChunksIndex; // the base texture index, added with popcount from unicodeIndexMask
    uint64_t* unicodeChunksMask; // each bit represents a single unicode

    Proportion_t* proportions; // array of UIFontProportion_v7_t
    Glyph_t* glyphs; // array of UIFontTexture_v7_t

    KerningInfo_t* kerningInfo; // for kerning?
};

struct UIFontAtlasAssetHeader_v6_t
{
    uint16_t fontCount;
    uint16_t unk_2; // count for the data at unk_18

    uint16_t width;
    uint16_t height;

    // like ui image atlas
    float widthRatio;
    float heightRatio;
    UIFontHeader_v7_t* fonts;
    uint8_t* unk_18;

    uint64_t atlasGUID; // guid
};

// S21 live font atlas is v12 (0x68 header).
struct UIFontTexture_v12_t
{
    float unk_0;
    uint16_t unk_4;
    uint16_t unk_6;
    char unk_8;
    char proportionIndex;
    uint8_t unk_A[2];
    float posBaseX;
    float posBaseY;
    float posMinX;
    float posMinY;
    float posMaxX;
    float posMaxY;
};

struct UIFontHeader_v12_t
{
    char* name;
    uint16_t fontIndex;
    uint16_t numProportions;
    uint16_t numGlyphChunks;
    uint16_t numUnicodeChunks;
    int glyphIndex;
    int unicodeIndex;
    uint32_t numGlyphTextures;
    uint32_t numUnicodeTextures;
    float proportionScaleX;
    float proportionScaleY;
    float unk_28[2];
    uint32_t textureIndex;
    int errorGlyph;
    uint16_t* glyphChunks;
    uint16_t* unicodeChunks;
    uint16_t* glyphChunksIndex;
    uint16_t* unicodeChunksIndex;
    uint64_t* glyphChunksMask;
    uint64_t* unicodeChunksMask;
    Proportion_t* proportions;
    UIFontTexture_v12_t* glyphTextures;
    UIFontTexture_v12_t* unicodeTextures;
    void* unk_80;
    void* unk_88;
};
static_assert(sizeof(UIFontHeader_v12_t) == 0x90, "UIFontHeader_v12_t");

struct UIFontAtlasAssetHeader_v12_t
{
    uint16_t fontCount;
    uint16_t unk_2;
    uint16_t width;
    uint16_t height;
    float widthRatio;
    float heightRatio;
    char unk_10[40];
    void* fonts;
    void* unk_40;
    uint64_t atlasGUID;
    void* unk_50;
    void* unk_58;
    int unk_60[2];
};
static_assert(sizeof(UIFontAtlasAssetHeader_v12_t) == 0x68, "UIFontAtlasAssetHeader_v12_t");

struct Font_t {
    std::string name;
    uint16_t fontIndex;
    float proportionScaleX;
    float proportionScaleY;
    float float_24;
    float float_28;
    int unicodeIndex;
    size_t baseTextureIndex;
    std::vector<Proportion_t> proportions;
    std::vector<KerningInfo_t> kerningInfos;
    std::vector<Glyph_t> glyphs;
    std::vector<uint16_t> unicodeChunk;
    std::vector<GlyphChunk_t> glyphChunks;

};




struct FontAtlas_t {

    uint16_t unk_2;
    uint16_t width;
    uint16_t height;

    float widthRatio;
    float heightRatio;

    std::map<uint16_t,Font_t> fonts;
    std::vector<uint8_t> unk_18;

    size_t textureId;
    size_t shaderDataId;

    FontAtlas_t(fs::path& jsonPath,size_t atlasIndex);
    FontAtlas_t(UIFontAtlasAssetHeader_v6_t* fontAtlasHdr,size_t textureId);
    FontAtlas_t(UIFontAtlasAssetHeader_v12_t* fontAtlasHdr,size_t textureId);
    void loadFromFile(fs::path& jsonPath);
    void CreateShaderDataBuffer();
};

extern std::vector<FontAtlas_t> fonts;
extern std::map<uint16_t, size_t> fontAtlasIndices;

// Engine face index: GetFontIndex(StringToGuid(name)) — same as rpak UIFontHeader.fontIndex.
uint16_t FontFaceIndexFromName(const char* name);
// Resolve by exact loaded name, then by engine face index (ArameMono etc.).
Font_t* getFontByName(const char* name);
void AssignEngineFontNames(Font_t& font);

void loadFonts();
Font_t* getFontByIndex(uint16_t id);
FontAtlas_t* getFontAtlasByIndex(uint16_t id);
void loadRpakFont(UIFontAtlasAssetHeader_v6_t* font, size_t textureId);
void loadRpakFontV12(UIFontAtlasAssetHeader_v12_t* font, size_t textureId);
void clearFontAtlases();