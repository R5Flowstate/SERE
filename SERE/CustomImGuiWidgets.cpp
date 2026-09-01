#include "CustomImGuiWidgets.h"
#include "Imgui/imgui_stdlib.h"

#include "Util.h"

#include "Imgui/implot.h"
#undef min
#undef max


bool AtlasImageButton(const char* id, uint32_t hash,ImVec2 maxSize) {
	if (hash == INVALID_ASSET || !imageAssetMap.contains(hash)) {
		// Missing asset (common under S21 UIIA until names resolve) — no insert/no crash.
		ImGui::Button(id ? id : "missing", ImVec2(maxSize.x > 0 ? maxSize.x : 64.f, 32.f));
		return false;
	}
	return AtlasImageButton(id, imageAssetMap.at(hash), maxSize);
}

bool AtlasImageButton(const char* id, const Asset_t& asset,ImVec2 maxSize) {
	if (imageAtlases.empty() || asset.atlasIndex >= imageAtlases.size()) {
		ImGui::Button(id ? id : "bad-atlas", ImVec2(64.f, 32.f));
		return false;
	}
	const ImageAtlas& atlas = imageAtlases[asset.atlasIndex];
	if (asset.imageIndex >= atlas.dimentions.size() || asset.imageIndex >= atlas.shaderData.size()) {
		ImGui::Button(id ? id : "bad-img", ImVec2(64.f, 32.f));
		return false;
	}
	const auto& dim = atlas.dimentions[asset.imageIndex];
	const auto& shd = atlas.shaderData[asset.imageIndex];
	if (dim.width == 0) {
		ImGui::Button(id ? id : "zero-w", ImVec2(64.f, 32.f));
		return false;
	}
	void* view = atlas.GetImageView();
	if (!view) {
		ImGui::Button(id ? id : "no-tex", ImVec2(64.f, 32.f));
		return false;
	}
	const ImVec2 mins(shd.minX, shd.minY);
	const ImVec2 maxs(shd.minX + shd.sizeX, shd.minY + shd.sizeY);
	ImVec2 displaySize(maxSize.x, maxSize.x / (float)dim.width * (float)dim.height);
	if (displaySize.y > maxSize.y) displaySize.y = maxSize.y;
	const char* label = asset.name.empty() ? (id ? id : "asset") : asset.name.c_str();
	return ImGui::ImageButton(label, (ImTextureRef)view, displaySize, mins, maxs);
}


void AssetSelectionPopup(const char* id, uint32_t* hash) {
	static std::string search = "";
	if (ImGui::BeginPopup("Asset Selection", ImGuiWindowFlags_MenuBar)) {

		if (ImGui::BeginMenuBar()) {
			ImGui::Text("Search:");
			ImGui::SameLine();
			ImGui::InputText("", &search);
			ImGui::EndMenuBar();
		}


		// Cap draw count — full S21 uiia map is ~8k entries; drawing all AVs/hangs.
		int shown = 0;
		const int kMaxShown = 64;
		ImGui::BeginTable("Assets", 6);
		for (const auto& [assetHash, asset] : imageAssetMap) {
			if (search.size() && !caseInsensitiveSearch(asset.name, search))
				continue;
			if (shown >= kMaxShown) {
				ImGui::TableNextColumn();
				ImGui::TextDisabled("... truncated (search to filter)");
				break;
			}
			ImGui::TableNextColumn();
			if (AtlasImageButton(asset.name.c_str(), asset)) {
				*hash = assetHash;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 100.f);
			ImGui::Text("%s", asset.name.c_str());
			ImGui::PopTextWrapPos();
			shown++;
		}
		ImGui::EndTable();
		ImGui::EndPopup();
	}
}

bool Slider2D(const char* id,float* xVal,float* yVal) {
	bool clicked = false,hovered = false,held = false;

	if (ImPlot::BeginPlot(id, ImVec2(150, 150), ImPlotFlags_CanvasOnly)) {
		const uint32_t axisFlags = ImPlotAxisFlags_NoTickMarks;
		ImPlot::SetupAxes(nullptr, nullptr, axisFlags, axisFlags);
		ImPlot::SetupAxesLimits(-1,1,-1,1);
		double x = *xVal;
		double y = *yVal;
		ImPlot::DragPoint(0, &x, &y, ImVec4(0, 0.9f, 0, 1), 7, 0, &clicked, &hovered, &held);
		*xVal = static_cast<float>(x);
		*yVal = static_cast<float>(y);
		ImPlot::EndPlot();
		if (ImGui::IsItemHovered()) {
			return true;
		}
	}
	return false;
}







void MappingCreationPopup(const char* id, float currentX, Mapping& map) {
	if(ImGui::BeginPopup(id)) {
		map.ShowEditUi(currentX);
		ImGui::EndPopup();
	}
}