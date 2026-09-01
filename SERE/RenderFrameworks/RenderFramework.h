#pragma once

#include <filesystem>
#include <vector>
#include <cstdint>
#include <memory>
#include "Util.h"
#include "ShaderStructs.h"
namespace fs = std::filesystem;

struct Vertex_t
{
	float position[3];
	float float_C[3];
	float float_18[4];

	float float_28[2];
	uint16_t assetIndex;
	uint16_t assetIndex2;
	uint16_t word_34;
	uint16_t word_36;
};



// Must match UiShaderStyle_s in ui_ps.fxc (160 bytes, stride=160)
struct StyleDescriptorShader_t
{
	Color color0 = Color(1.f,1.f,1.f,1.f);         // offset 0
	Color color1 = Color(0.f,0.f,0.f,0.f);         // offset 16
	Color color2 = Color(0.f,0.f,0.f,0.f);         // offset 32
	Color colorxfrm0 = Color(1.f,0.f,0.f,0.f);     // offset 48
	Color colorxfrm1 = Color(0.f,1.f,0.f,0.f);     // offset 64
	Color colorxfrm2 = Color(0.f,0.f,1.f,0.f);     // offset 80
	Color tint = Color(1.f,1.f,1.f,1.f);            // offset 96
	float blend = 1.f;                               // offset 112
	float premul = 0.f;                              // offset 116
	float _anon_0 = 0.f;                            // offset 120
	float _anon_1 = 0.f;                            // offset 124
	float _anon_2 = 0.f;                            // offset 128
	float _anon_3 = 0.f;                            // offset 132
	float _anon_4 = 0.f;                            // offset 136
	float _anon_5 = 0.f;                            // offset 140
	float _anon_6 = 0.f;                            // offset 144
	float desaturate = 0.f;                          // offset 148
	float hueShift = 0.f;                            // offset 152
	float lightness = 0.f;                           // offset 156
};

struct ShaderSizeData_t {
	float minX;
	float minY;
	float sizeX;
	float sizeY;
};


class RenderFramework {

public:

	RenderFramework() {};

	virtual bool ShouldMainLoopRun() = 0;

	virtual bool ImGuiStartFrame() = 0;
	virtual void ImGuiEndFrame() = 0;

	virtual void ImGuiDeInit() = 0;

	virtual size_t LoadTexture(fs::path& path) = 0;
	virtual size_t CreateTextureFromData(void* data,uint32_t width,uint32_t height,uint16_t format,uint32_t pitch,uint32_t slicePitch) = 0;
	virtual size_t CreateShaderDataBuffer(std::vector<ShaderSizeData_t> data) = 0;

	virtual void RuiWriteIndexBuffer(std::vector<uint16_t>& data) = 0;
	virtual void RuiWriteVertexBuffer(std::vector<Vertex_t>& data) = 0;
	virtual void RuiWriteStyleBuffer(std::vector<StyleDescriptorShader_t>& data) = 0;

	virtual void RuiClearFrame() = 0;


	virtual void DrawIndexed(uint32_t count,uint32_t start,size_t* resources) = 0;

	virtual void RuiBindPipeline() = 0;
	virtual void RuiLoad(int width,int height) = 0;
	virtual void RuiReCreatePipeline(int width,int height) = 0;

	virtual void* GetTextureView(size_t id) = 0;
	virtual void* GetRuiView() = 0;

	// Reads the offscreen RUI target back to a PNG. Empty result means the
	// backend cannot read back; callers must handle that.
	virtual bool CapturePreviewPng(std::vector<uint8_t>& out, int& width, int& height) { return false; }

};

void CreateRenderFramework(char** argv,int argc);
extern std::unique_ptr<RenderFramework> g_renderFramework;
