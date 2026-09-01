

#include "FontAtlas.h"

#include <fstream>
#include <mutex>
#include "ThirdParty/DDSTextureLoader11.h"
#include "ThirdParty/rapidjson/document.h"
#include "PakLoading/utils.h"


#undef GetObject

std::vector<FontAtlas_t> fonts{};
std::map<uint16_t, size_t> fontAtlasIndices{};
std::mutex fontMutex{};

// A shipped rpak UIFontHeader.name is often null; the engine resolves faces by
// GetFontIndex(StringToGuid(name)) == fontIndex (RSX ui_font_atlas.h).
uint16_t FontFaceIndexFromName(const char* name)
{
	if (!name || !*name)
		return 0xFFFF;
	const uint64_t h = RTech::StringToGuid(name);
	return static_cast<uint16_t>((h ^ (h >> 11)) & 0x1F);
}

// Names that appear in RUI JSON / ui.dll (StringToGuid -> face index 0..31).
static const char* kEngineFontNames[] = {
	"ArameMono",
	"ArameMonoThin",
	"DefaultRegularFont",
	"DefaultBoldFont",
	"DefaultItalicFont",
	"DefaultBoldItalicFont",
	"Titanfall",
	"TitanfallBold",
	"Arial",
	"ArialBold",
	"CourierNew",
	"TimesNewRoman",
};

void AssignEngineFontNames(Font_t& font)
{
	// Keep a real string from the pak when present.
	if (!font.name.empty() && font.name.rfind("Font_", 0) != 0 && font.name.rfind("font_", 0) != 0)
		return;
	for (const char* n : kEngineFontNames) {
		if (FontFaceIndexFromName(n) == font.fontIndex) {
			font.name = n;
			return;
		}
	}
	// Still unnamed — keep index-stable label for UI, not used for face lookup.
	if (font.name.empty())
		font.name = std::format("font_{}", font.fontIndex);
}

Font_t* getFontByName(const char* name)
{
	if (!name || !*name)
		return nullptr;
	for (auto& atlas : fonts) {
		for (auto& [idx, font] : atlas.fonts) {
			if (font.name == name)
				return &font;
		}
	}
	// Engine path: name -> face index -> loaded face.
	return getFontByIndex(FontFaceIndexFromName(name));
}


struct ProportionData {
    float x;
    float y;
};


void FontAtlas_t::loadFromFile(fs::path& jsonPath) {
    std::ifstream jsonFile{jsonPath};
    if(jsonFile.fail())return;
    std::stringstream jsonStringStream;
    while (jsonFile.peek() != EOF)
        jsonStringStream << (char)jsonFile.get();
    jsonFile.close();

    rapidjson::Document doc;
    doc.Parse(jsonStringStream.str().c_str());
    if(doc.HasParseError())return;
    
    rapidjson::GenericObject root = doc.GetObject();



    if(!(root.HasMember("width")&&root["width"].IsUint()))return;
    if(!(root.HasMember("height")&&root["height"].IsUint()))return;
    if(!(root.HasMember("widthRatio")&&root["widthRatio"].IsNumber()))return; 
    if(!(root.HasMember("heightRatio")&&root["heightRatio"].IsNumber()))return;
    if(!(root.HasMember("unk_2")&&root["unk_2"].IsUint()))return;
    if((!root.HasMember("fonts")&&root["fonts"].IsArray()))return;
    if(!(root.HasMember("unk_18")&&root["unk_18"].IsArray()))return;

    width = root["width"].GetUint();
    widthRatio = root["widthRatio"].GetFloat();
    height = root["height"].GetUint();
    heightRatio = root["heightRatio"].GetFloat();
    unk_2 = root["unk_2"].GetUint();
    
    rapidjson::GenericArray fontArray = root["fonts"].GetArray();

    for(auto fontItr = fontArray.Begin();fontItr != fontArray.End();fontItr++) {
        if(!fontItr->IsObject())continue;
        rapidjson::GenericObject fontObj = fontItr->GetObject();

        if(!(fontObj.HasMember("name")&&fontObj["name"].IsString()))continue;
        if(!(fontObj.HasMember("fontId")&&fontObj["fontId"].IsInt()))continue;
        if(!(fontObj.HasMember("proportionScaleX")&&fontObj["proportionScaleX"].IsNumber()))continue;
        if(!(fontObj.HasMember("proportionScaleY")&&fontObj["proportionScaleY"].IsNumber()))continue;
        if(!(fontObj.HasMember("unk_24")&&fontObj["unk_24"].IsNumber()))continue;
        if(!(fontObj.HasMember("unk_28")&&fontObj["unk_28"].IsNumber()))continue;
        if(!(fontObj.HasMember("unicodeIndex")&&fontObj["unicodeIndex"].IsInt()))continue;
        if(!(fontObj.HasMember("unicodeChunks")&&fontObj["unicodeChunks"].IsArray()))continue;
        if(!(fontObj.HasMember("glyphChunks")&&fontObj["glyphChunks"].IsArray()))continue;
        if(!(fontObj.HasMember("proportions")&&fontObj["proportions"].IsArray()))continue;
        if(!(fontObj.HasMember("glyphs")&&fontObj["glyphs"].IsArray()))continue;
        if(!(fontObj.HasMember("KerningInfo")&&fontObj["KerningInfo"].IsArray()))continue;
        
        

        Font_t font;
        font.name = fontObj["name"].GetString();
        font.fontIndex = fontObj["fontId"].GetInt();
        font.proportionScaleX = fontObj["proportionScaleX"].GetFloat();
        font.proportionScaleY = fontObj["proportionScaleY"].GetFloat();
        font.float_24 = fontObj["unk_24"].GetFloat();
        font.float_28 = fontObj["unk_28"].GetFloat();
        font.unicodeIndex = fontObj["unicodeIndex"].GetInt();
        rapidjson::GenericArray unicodeChunks = fontObj["unicodeChunks"].GetArray();
        for(auto itr = unicodeChunks.Begin();itr != unicodeChunks.End();itr++){
            if(!itr->IsInt())continue;
            font.unicodeChunk.push_back(itr->GetInt());
        }
        rapidjson::GenericArray glyphCh = fontObj["glyphChunks"].GetArray();
        for(auto itr = glyphCh.Begin();itr != glyphCh.End();itr++) {
            if(!itr->IsObject())continue;
            rapidjson::GenericObject ch = itr->GetObject();

            if(!(ch.HasMember("unicodeChunksIndex")&&ch["unicodeChunksIndex"].IsInt()))continue;
            if(!(ch.HasMember("mask")&&ch["mask"].IsUint64()))continue;

            GlyphChunk_t chunk;
            chunk.glyphIndex = ch["unicodeChunksIndex"].GetInt();
            chunk.mask = ch["mask"].GetUint64();
            font.glyphChunks.push_back(chunk);
        }
        rapidjson::GenericArray props = fontObj["proportions"].GetArray();
        for(auto itr = props.Begin();itr != props.End();itr++) {
            if(!itr->IsObject())continue;
            rapidjson::GenericObject propObj = itr->GetObject();

            if(!(propObj.HasMember("scaleBounds")&&propObj["scaleBounds"].IsNumber()))continue;
            if(!(propObj.HasMember("scaleSize")&&propObj["scaleSize"].IsNumber()))continue;

            Proportion_t prop;
            prop.scaleBounds = propObj["scaleBounds"].GetFloat();
            prop.scaleSize = propObj["scaleSize"].GetFloat();
            font.proportions.push_back(prop);
        }
        rapidjson::GenericArray glyphs = fontObj["glyphs"].GetArray();
        for(auto itr = glyphs.Begin();itr != glyphs.End();itr++) {
            if(!itr->IsObject())continue;
            rapidjson::GenericObject glyphObj = itr->GetObject();

            if(!(glyphObj.HasMember("unk_0")&&glyphObj["unk_0"].IsNumber()))continue;
            if(!(glyphObj.HasMember("kerningBaseIndex")&&glyphObj["kerningBaseIndex"].IsInt()))continue;
            if(!(glyphObj.HasMember("unk_6")&&glyphObj["unk_6"].IsInt()))continue;
            if(!(glyphObj.HasMember("proportionIndex")&&glyphObj["proportionIndex"].IsInt()))continue;
            if(!(glyphObj.HasMember("posBaseX")&&glyphObj["posBaseX"].IsNumber()))continue;
            if(!(glyphObj.HasMember("posBaseY")&&glyphObj["posBaseY"].IsNumber()))continue;
            if(!(glyphObj.HasMember("posMinX")&&glyphObj["posMinX"].IsNumber()))continue;
            if(!(glyphObj.HasMember("posMinY")&&glyphObj["posMinY"].IsNumber()))continue;
            if(!(glyphObj.HasMember("posMaxX")&&glyphObj["posMaxX"].IsNumber()))continue;
            if(!(glyphObj.HasMember("posMaxY")&&glyphObj["posMaxY"].IsNumber()))continue;
            Glyph_t glyph;
            glyph.float_0 = glyphObj["unk_0"].GetFloat();
            glyph.kerningStartIndex = glyphObj["kerningBaseIndex"].GetInt();
            glyph.byte_6 = glyphObj["unk_6"].GetInt();
            glyph.proportionIndex = glyphObj["proportionIndex"].GetInt();
            glyph.posBaseX = glyphObj["posBaseX"].GetFloat();
            glyph.posBaseY = glyphObj["posBaseY"].GetFloat();
            glyph.posMinX = glyphObj["posMinX"].GetFloat();
            glyph.posMinY = glyphObj["posMinY"].GetFloat();
            glyph.posMaxX = glyphObj["posMaxX"].GetFloat();
            glyph.posMaxY = glyphObj["posMaxY"].GetFloat();
            font.glyphs.push_back(glyph);
        }
        rapidjson::GenericArray kernings = fontObj["KerningInfo"].GetArray();
        for(auto itr = kernings.Begin();itr != kernings.End();itr++) {
            if(!itr->IsObject())continue;
            rapidjson::GenericObject kernObj = itr->GetObject();
            if(!(kernObj.HasMember("otherIndex")&&kernObj["otherIndex"].IsInt()))continue;
            if(!(kernObj.HasMember("distance")&&kernObj["distance"].IsNumber()))continue;

            KerningInfo_t kern;
            kern.otherChar = kernObj["otherIndex"].GetInt();
            kern.kerningDistance = kernObj["distance"].GetFloat();
            font.kerningInfos.push_back(kern);
        }
        fonts.emplace(font.fontIndex,font);
    }
    rapidjson::GenericArray unk18 = root["unk_18"].GetArray();
    for(auto itr = unk18.Begin();itr != unk18.End();itr++) {
        if(!itr->IsInt())continue;
        unk_18.push_back(itr->GetInt());
    }
    

    fs::path ddsName = jsonPath.replace_extension("dds");

    textureId = g_renderFramework->LoadTexture(ddsName);

    CreateShaderDataBuffer();

}


FontAtlas_t::FontAtlas_t(fs::path& jsonPath,size_t atlasIndex) { 
    loadFromFile(jsonPath); 
    for (auto& fnt : fonts) {
        fontAtlasIndices.emplace(fnt.first,atlasIndex);
    }
};
#undef max
FontAtlas_t::FontAtlas_t(UIFontAtlasAssetHeader_v6_t* fontAtlasHdr,size_t texture):
    textureId(texture)

{
	width = fontAtlasHdr->width;
	height = fontAtlasHdr->height;
	widthRatio = fontAtlasHdr->widthRatio;
	heightRatio = fontAtlasHdr->heightRatio;
	unk_2 = fontAtlasHdr->unk_2;

    uint8_t highest_unk_6 = 0;
    for (uint16_t i = 0; i < fontAtlasHdr->fontCount; i++) {
        Font_t font;
        UIFontHeader_v7_t* fontHdr = &fontAtlasHdr->fonts[i];
        if(fontHdr->name && reinterpret_cast<uintptr_t>(fontHdr->name) > 0x10000)
            font.name = fontHdr->name;
        font.fontIndex = fontHdr->fontIndex;
		AssignEngineFontNames(font);
        font.unicodeIndex = fontHdr->unicodeIndex;
        font.proportionScaleX = fontHdr->proportionScaleX;
        font.proportionScaleY = fontHdr->proportionScaleY;
        font.float_24 = fontHdr->unk_24;
        font.float_28 = fontHdr->unk_28;

        font.proportions.resize(fontHdr->numProportions);
        memcpy(font.proportions.data(),fontHdr->proportions,sizeof(Proportion_t)*fontHdr->numProportions);
        
        uint16_t highestUnicodeIndex = 0;
        for (uint16_t j = 0; j < fontHdr->numUnicodeChunks; j++) {
            font.unicodeChunk.push_back(fontHdr->unicodeChunks[j]);
            highestUnicodeIndex = std::max(highestUnicodeIndex,fontHdr->unicodeChunks[j]);
        }
        for (uint16_t j = 0; j <= highestUnicodeIndex; j++) {
            font.glyphChunks.emplace_back(fontHdr->unicodeChunksIndex[j],fontHdr->unicodeChunksMask[j]);
        }
        for (uint16_t j = 0; j < fontHdr->numTextures; j++) {
            font.glyphs.push_back(fontHdr->glyphs[j]);
            highest_unk_6 = std::max(highest_unk_6,fontHdr->glyphs[j].byte_6);
        }
		if (!font.glyphs.empty()) {
			Glyph_t sent{};
			sent.kerningStartIndex = font.glyphs.back().kerningStartIndex;
			font.glyphs.push_back(sent);
		}

        font.kerningInfos.resize(fontHdr->numKernings);
		if (fontHdr->kerningInfo && fontHdr->numKernings)
			memcpy(font.kerningInfos.data(),fontHdr->kerningInfo,sizeof(KerningInfo_t)*fontHdr->numKernings);
        fonts.insert({ font.fontIndex,font });
    }
    uint16_t numUnk18 = ((fontAtlasHdr->unk_2 + 1) * highest_unk_6 * 2 + 1) >> 3;
    for (uint16_t i = 0; i < numUnk18; i++) {
        unk_18.push_back(fontAtlasHdr->unk_18[i]);
    }
    CreateShaderDataBuffer();
    
}

void FontAtlas_t::CreateShaderDataBuffer() {

    std::vector<ShaderSizeData_t> shaderData;
    for (auto& font : fonts)
    {

        font.second.baseTextureIndex = shaderData.size();
        std::vector<ProportionData> propA;
        std::vector<ProportionData>  propB;

        for (auto& proportion : font.second.proportions) {

            ProportionData a;
            a.x = proportion.scaleBounds * font.second.proportionScaleX;
            a.y = proportion.scaleBounds * font.second.proportionScaleY;
            propA.push_back(a);

            ProportionData b;
            b.x = proportion.scaleSize * a.x;
            b.y = proportion.scaleSize * a.y;
            propB.push_back(b);
        }

        for (auto& glyph : font.second.glyphs)
        {

            ShaderSizeData_t shdDat;

            if ((glyph.posMinX == glyph.posMaxX) || (glyph.proportionIndex >= font.second.proportions.size()))
            {
                shdDat.minX = 1.f;
                shdDat.minY = 1.f;
                shdDat.sizeX = 0.f;
                shdDat.sizeY = 0.f;
            }
            else
            {
                uint8_t propIndex = glyph.proportionIndex;

                shdDat.minX = glyph.posMinX * propA[propIndex].x + glyph.posBaseX - propB[propIndex].x;
                shdDat.minY = glyph.posMinY * propA[propIndex].y + glyph.posBaseY - propB[propIndex].y;

                shdDat.sizeX = glyph.posMaxX * propA[propIndex].x + glyph.posBaseX + propB[propIndex].x;
                shdDat.sizeY = glyph.posMaxY * propA[propIndex].y + glyph.posBaseY + propB[propIndex].y;
            }
            shaderData.push_back(shdDat);
        }
    }

    if (shaderData.size()) {
        shaderDataId = g_renderFramework->CreateShaderDataBuffer(shaderData);
    }
    else {
        shaderDataId = ~0ull;
    }
}


void loadFonts() {
    fs::path folderPath = ".\\Assets\\Fonts";
	std::error_code ec;
	if (!fs::exists(folderPath, ec))
		return;

    for (const auto& dirEntry : fs::recursive_directory_iterator(folderPath, ec)) {
		if (ec) break;
        if(!dirEntry.is_regular_file())continue;
        fs::path jsonName = dirEntry;
        if(jsonName.extension()!=".json")continue;
        fontMutex.lock();
        size_t fontId = fonts.size();
        fonts.emplace_back(jsonName,fontId);
		for (auto& [k, v] : fonts.back().fonts) {
			AssignEngineFontNames(v);
			fontAtlasIndices[k] = fontId;
		}
        fontMutex.unlock();
    }
}




#undef max
void loadRpakFont(UIFontAtlasAssetHeader_v6_t* fontAtlasHdr, size_t textureId) {
   printf("[SERE] loadRpakFont: fontCount=%d, textureId=%zu\n", fontAtlasHdr->fontCount, textureId); fflush(stdout);
   fontMutex.lock();
   size_t atlasIdx = fonts.size();
   fonts.emplace_back(fontAtlasHdr,textureId);
   for (auto& [k, v] : fonts.back().fonts) {
       fontAtlasIndices.emplace(k, atlasIdx);
   }
   printf("[SERE] loadRpakFont: registered atlas %zu, fonts in atlas:", atlasIdx);
   for (auto& [k, v] : fonts.back().fonts) printf(" %d('%s')", k, v.name.c_str());
   printf("\n"); fflush(stdout);
   fontMutex.unlock();
}

FontAtlas_t::FontAtlas_t(UIFontAtlasAssetHeader_v12_t* fontAtlasHdr, size_t texture) :
    textureId(texture)
{
	width = fontAtlasHdr->width;
	height = fontAtlasHdr->height;
	widthRatio = fontAtlasHdr->widthRatio;
	heightRatio = fontAtlasHdr->heightRatio;
	unk_2 = fontAtlasHdr->unk_2;

    auto* fontArr = reinterpret_cast<UIFontHeader_v12_t*>(fontAtlasHdr->fonts);
    if (!fontArr) {
        CreateShaderDataBuffer();
        return;
    }
    uint8_t highest_unk_6 = 0;
    for (uint16_t i = 0; i < fontAtlasHdr->fontCount; i++) {
        Font_t font;
        UIFontHeader_v12_t* fontHdr = &fontArr[i];
		// name is PagePtr-fixed by ResolvePointers; often null -- face id is authoritative.
        if (fontHdr->name && reinterpret_cast<uintptr_t>(fontHdr->name) > 0x10000)
            font.name = fontHdr->name;
        font.fontIndex = fontHdr->fontIndex;
		AssignEngineFontNames(font);
        font.unicodeIndex = fontHdr->unicodeIndex;
        font.proportionScaleX = fontHdr->proportionScaleX;
        font.proportionScaleY = fontHdr->proportionScaleY;
        font.float_24 = fontHdr->unk_28[0];
        font.float_28 = fontHdr->unk_28[1];

        font.proportions.resize(fontHdr->numProportions);
        if (fontHdr->proportions && fontHdr->numProportions)
            memcpy(font.proportions.data(), fontHdr->proportions, sizeof(Proportion_t) * fontHdr->numProportions);

		// Unicode path (getFontGlyphIndex): same layout as v6/v7 walk.
        uint16_t highestUnicodeIndex = 0;
        if (fontHdr->unicodeChunks && fontHdr->numUnicodeChunks) {
            for (uint16_t j = 0; j < fontHdr->numUnicodeChunks; j++) {
                font.unicodeChunk.push_back(fontHdr->unicodeChunks[j]);
                highestUnicodeIndex = std::max(highestUnicodeIndex, fontHdr->unicodeChunks[j]);
            }
        }
        if (fontHdr->unicodeChunksIndex && fontHdr->unicodeChunksMask && !font.unicodeChunk.empty()) {
            for (uint16_t j = 0; j <= highestUnicodeIndex; j++) {
                font.glyphChunks.emplace_back(fontHdr->unicodeChunksIndex[j], fontHdr->unicodeChunksMask[j]);
            }
        }
		// Glyph texture table: unicodeTextures preferred (matches v6 glyphs for measure/draw).
        const uint32_t nTex = fontHdr->numUnicodeTextures ? fontHdr->numUnicodeTextures : fontHdr->numGlyphTextures;
        UIFontTexture_v12_t* texArr = fontHdr->unicodeTextures ? fontHdr->unicodeTextures : fontHdr->glyphTextures;
        if (texArr && nTex) {
            for (uint32_t j = 0; j < nTex; j++) {
                Glyph_t g{};
                g.float_0 = texArr[j].unk_0;
                g.kerningStartIndex = texArr[j].unk_4;
                g.byte_6 = (uint8_t)texArr[j].unk_6;
                g.proportionIndex = (uint8_t)texArr[j].proportionIndex;
                g.posBaseX = texArr[j].posBaseX;
                g.posBaseY = texArr[j].posBaseY;
                g.posMinX = texArr[j].posMinX;
                g.posMinY = texArr[j].posMinY;
                g.posMaxX = texArr[j].posMaxX;
                g.posMaxY = texArr[j].posMaxY;
                font.glyphs.push_back(g);
                highest_unk_6 = std::max(highest_unk_6, g.byte_6);
            }
        }
		// Engine kerning end is glyphs[i+1].kerningStartIndex — need sentinel.
		if (!font.glyphs.empty()) {
			Glyph_t sent{};
			sent.kerningStartIndex = font.glyphs.back().kerningStartIndex;
			font.glyphs.push_back(sent);
		}
        fonts.insert({ font.fontIndex, font });
    }
	// unk_40 is v12's former unk_18 bitfield (atlas-level).
	if (fontAtlasHdr->unk_40 && highest_unk_6) {
		const uint16_t numUnk = ((fontAtlasHdr->unk_2 + 1) * highest_unk_6 * 2 + 1) >> 3;
		auto* bits = reinterpret_cast<uint8_t*>(fontAtlasHdr->unk_40);
		for (uint16_t i = 0; i < numUnk; i++)
			unk_18.push_back(bits[i]);
	}
    CreateShaderDataBuffer();
}

void loadRpakFontV12(UIFontAtlasAssetHeader_v12_t* fontAtlasHdr, size_t textureId) {
    printf("[SERE] loadRpakFontV12: fontCount=%d, textureId=%zu\n", fontAtlasHdr->fontCount, textureId); fflush(stdout);
    fontMutex.lock();
    size_t atlasIdx = fonts.size();
    fonts.emplace_back(fontAtlasHdr, textureId);
    for (auto& [k, v] : fonts.back().fonts) {
        fontAtlasIndices[k] = atlasIdx; // last writer wins (same as engine face list)
    }
    printf("[SERE] loadRpakFontV12: registered atlas %zu fonts:", atlasIdx);
    for (auto& [k, v] : fonts.back().fonts) printf(" %d('%s')", k, v.name.c_str());
    printf("\n"); fflush(stdout);
    fontMutex.unlock();
}

Font_t* getFontByIndex(uint16_t id) {
    if (fontAtlasIndices.count(id) == 0)
        return nullptr;
    size_t atlasIdx = fontAtlasIndices[id];
    if (atlasIdx >= fonts.size())
        return nullptr;
    if (fonts[atlasIdx].fonts.count(id) == 0)
        return nullptr;
    return &fonts[atlasIdx].fonts[id];
}
FontAtlas_t* getFontAtlasByIndex(uint16_t id) {
    if (fontAtlasIndices.count(id) == 0)
        return nullptr;
    size_t atlasIdx = fontAtlasIndices[id];
    if (atlasIdx >= fonts.size())
        return nullptr;
    return &fonts[atlasIdx];
}

void clearFontAtlases() {
    fontMutex.lock();
    fontAtlasIndices.clear();
    fonts.clear();
    fontMutex.unlock();
}