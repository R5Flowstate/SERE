// SERE.cpp : Defines the entry point for the application.
//

#include <vector>
#include "Bridge/ControlServer.h"
#include <fstream>
#include <streambuf>
#include <execution>

#include "SERE.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "Imgui/implot.h"

#include "RenderFrameworks/RenderFramework_Dx11.h"
#include "RuiNodeEditor/RuiNodeEditor.h"

#include "Nodes/ArgumentNodes.h"
#include "Nodes/ConstantVarNodes.h"
#include "Nodes/GlobalNodes.h"
#include "Nodes/MathNodes.h"
#include "Nodes/RenderJobNodes.h"
#include "Nodes/SplitMergeNodes.h"
#include "Nodes/TransformNodes.h"
#include "Nodes/ConditionalNodes.h"

#include "ThirdParty/nativefiledialog-extended/src/include/nfd.hpp"

#include "Settings.h"
#include "PakLoading/cpakfile.h"



static void ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
    ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("click here"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ShowExampleAppDockSpace(bool* p_open)
{
    // If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
    // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
    // In this specific demo, we are not using DockSpaceOverViewport() because:
    // - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
    // - we allow the host window to have padding (when opt_padding == true)
    // - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
    // TL;DR; this demo is more complicated than what you would normally use.
    // If we removed all the options we are showcasing, this demo would become:
    //     void ShowExampleAppDockSpace()
    //     {
    //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //     }

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        ShowDockingDisabledMessage();
    }

    ImGui::End();
}

void ReloadAssets(std::string folderPath) {
    printf("[SERE] ReloadAssets called with path: '%s'\n", folderPath.c_str());

    static std::string loadedPath = "";
    if(loadedPath == folderPath || folderPath.empty()) {
        printf("[SERE] Path unchanged or empty, skipping reload\n"); fflush(stdout);
        return;
    }
    loadedPath = folderPath;
    printf("[SERE] clearImageAtlases...\n");
    clearImageAtlases();
    printf("[SERE] clearFontAtlases...\n");
    clearFontAtlases();
    printf("[SERE] loadFonts...\n");
    loadFonts();
    printf("[SERE] loadImageNameLookup...\n");
    loadImageNameLookup();
    printf("[SERE] loadImageAtlases...\n");
    loadImageAtlases();
    printf("[SERE] Starting rpak loading...\n");

    // Preview atlas sources. ui.rpak carries the stock UI; the SDK and flowstate
    // paks carry everything this build adds on top (rui/flowstate_custom/*),
    // so a graph referencing them previews with the real art instead of an
    // unresolved-asset fallback. SERE_EXTRA_PAKS appends more, ';'-separated.
    fs::path pakDir = fs::path(folderPath) / "paks" / "Win64";

    std::vector<std::string> pakNames = {
        "ui.rpak",
        "ui_sdk.rpak",
        "common_flowstate.rpak",
    };
    if (const char* extra = getenv("SERE_EXTRA_PAKS")) {
        std::string list = extra;
        size_t at = 0;
        while (at <= list.size()) {
            size_t next = list.find(';', at);
            if (next == std::string::npos) next = list.size();
            std::string one = list.substr(at, next - at);
            if (!one.empty()) pakNames.push_back(one);
            at = next + 1;
        }
    }

    int loaded = 0;
    for (const std::string& name : pakNames) {
        // A decompressed sibling still wins: it parses without Oodle at all.
        fs::path dec = pakDir / (name + ".dec.rpak");
        fs::path live = pakDir / name;
        fs::path fullPath;
        std::string label;
        if (fs::exists(dec)) {
            fullPath = dec;
            label = name + ".dec.rpak";
        } else if (fs::exists(live)) {
            fullPath = live;
            label = name;
        } else {
            printf("[SERE] Skipping (not present): %s\n", name.c_str());
            continue;
        }

        printf("[SERE] Loading: %s (%lld bytes)\n", label.c_str(), (long long)fs::file_size(fullPath));
        fflush(stdout);
        try {
            LoadRpak(fullPath);
            loaded++;
        } catch (const std::exception& e) {
            printf("[SERE] Exception loading %s: %s\n", label.c_str(), e.what());
        } catch (...) {
            printf("[SERE] Unknown exception loading %s\n", label.c_str());
        }
        printf("[SERE] Done: %s\n", label.c_str());
        fflush(stdout);
    }

    printf("[SERE] Pak load complete: %d/%zu paks, %zu image assets\n",
        loaded, pakNames.size(), imageAssetMap.size());
    fflush(stdout);
}

// Main code
int main(int argc, char** argv)
{
    // Redirect stdout to log file so we can see output even on crash
    freopen("sere_log.txt", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0); // unbuffered - every write goes to disk immediately

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    Settings settings;

    // SERE Dark Theme
    ImGui::StyleColorsDark();
    {
        ImGuiStyle& s = ImGui::GetStyle();
        // Window
        s.WindowRounding    = 4.0f;
        s.WindowBorderSize  = 1.0f;
        s.WindowPadding     = ImVec2(10, 10);
        s.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
        // Frame
        s.FrameRounding     = 3.0f;
        s.FramePadding      = ImVec2(6, 4);
        s.FrameBorderSize   = 0.0f;
        // Items
        s.ItemSpacing       = ImVec2(8, 5);
        s.ItemInnerSpacing  = ImVec2(5, 4);
        s.IndentSpacing     = 18.0f;
        // Widgets
        s.GrabRounding      = 2.0f;
        s.GrabMinSize       = 10.0f;
        s.ScrollbarRounding = 8.0f;
        s.ScrollbarSize     = 13.0f;
        s.TabRounding       = 3.0f;
        s.PopupRounding     = 4.0f;
        s.ChildRounding     = 3.0f;
        // Separator
        s.SeparatorTextBorderSize = 1.0f;

        ImVec4* c = s.Colors;
        // Base backgrounds
        c[ImGuiCol_WindowBg]             = ImVec4(0.106f, 0.110f, 0.125f, 1.00f);
        c[ImGuiCol_ChildBg]              = ImVec4(0.106f, 0.110f, 0.125f, 1.00f);
        c[ImGuiCol_PopupBg]              = ImVec4(0.118f, 0.122f, 0.141f, 0.98f);
        // Borders
        c[ImGuiCol_Border]               = ImVec4(0.200f, 0.212f, 0.247f, 0.60f);
        c[ImGuiCol_BorderShadow]         = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
        // Frames (input fields, checkboxes)
        c[ImGuiCol_FrameBg]              = ImVec4(0.153f, 0.161f, 0.188f, 1.00f);
        c[ImGuiCol_FrameBgHovered]       = ImVec4(0.196f, 0.208f, 0.243f, 1.00f);
        c[ImGuiCol_FrameBgActive]        = ImVec4(0.227f, 0.243f, 0.286f, 1.00f);
        // Title bar
        c[ImGuiCol_TitleBg]              = ImVec4(0.082f, 0.086f, 0.102f, 1.00f);
        c[ImGuiCol_TitleBgActive]        = ImVec4(0.118f, 0.125f, 0.153f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.082f, 0.086f, 0.102f, 0.80f);
        // Menu bar
        c[ImGuiCol_MenuBarBg]            = ImVec4(0.118f, 0.125f, 0.153f, 1.00f);
        // Scrollbar
        c[ImGuiCol_ScrollbarBg]          = ImVec4(0.082f, 0.086f, 0.102f, 0.60f);
        c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.243f, 0.255f, 0.298f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.310f, 0.325f, 0.376f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.376f, 0.396f, 0.459f, 1.00f);
        // Buttons - muted blue accent
        c[ImGuiCol_Button]               = ImVec4(0.204f, 0.224f, 0.298f, 1.00f);
        c[ImGuiCol_ButtonHovered]        = ImVec4(0.271f, 0.302f, 0.412f, 1.00f);
        c[ImGuiCol_ButtonActive]         = ImVec4(0.325f, 0.365f, 0.502f, 1.00f);
        // Headers (collapsing headers, menu items)
        c[ImGuiCol_Header]               = ImVec4(0.188f, 0.200f, 0.243f, 1.00f);
        c[ImGuiCol_HeaderHovered]        = ImVec4(0.247f, 0.267f, 0.337f, 1.00f);
        c[ImGuiCol_HeaderActive]         = ImVec4(0.302f, 0.329f, 0.420f, 1.00f);
        // Separator
        c[ImGuiCol_Separator]            = ImVec4(0.200f, 0.212f, 0.247f, 0.60f);
        c[ImGuiCol_SeparatorHovered]     = ImVec4(0.345f, 0.451f, 0.690f, 0.80f);
        c[ImGuiCol_SeparatorActive]      = ImVec4(0.345f, 0.451f, 0.690f, 1.00f);
        // Resize grip
        c[ImGuiCol_ResizeGrip]           = ImVec4(0.247f, 0.267f, 0.337f, 0.40f);
        c[ImGuiCol_ResizeGripHovered]    = ImVec4(0.345f, 0.451f, 0.690f, 0.67f);
        c[ImGuiCol_ResizeGripActive]     = ImVec4(0.345f, 0.451f, 0.690f, 0.95f);
        // Tabs
        c[ImGuiCol_Tab]                  = ImVec4(0.141f, 0.149f, 0.176f, 1.00f);
        c[ImGuiCol_TabHovered]           = ImVec4(0.247f, 0.267f, 0.337f, 1.00f);
        // Slider / Check
        c[ImGuiCol_CheckMark]            = ImVec4(0.502f, 0.631f, 0.878f, 1.00f);
        c[ImGuiCol_SliderGrab]           = ImVec4(0.376f, 0.467f, 0.667f, 1.00f);
        c[ImGuiCol_SliderGrabActive]     = ImVec4(0.463f, 0.561f, 0.776f, 1.00f);
        // Text
        c[ImGuiCol_Text]                 = ImVec4(0.882f, 0.894f, 0.925f, 1.00f);
        c[ImGuiCol_TextDisabled]         = ImVec4(0.427f, 0.447f, 0.506f, 1.00f);
        // Docking
        c[ImGuiCol_DockingPreview]       = ImVec4(0.345f, 0.451f, 0.690f, 0.70f);
        c[ImGuiCol_DockingEmptyBg]       = ImVec4(0.082f, 0.086f, 0.102f, 1.00f);
    }

    CreateRenderFramework(argv,argc);
    auto ruiSize = settings.GetRuiSize();
    g_renderFramework->RuiLoad(ruiSize.width,ruiSize.height);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    ImFontConfig config;
    config.OversampleH = 2;
    io.Fonts->AddFontFromFileTTF("imgui/DroidSans.ttf", 16.0f,&config);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    bool use_docking_space = false;

    ImVec4 clear_color = ImVec4(0.082f, 0.086f, 0.102f, 1.00f);

    RenderInstance render{(float)ruiSize.width,(float)ruiSize.height};
    NodeEditor nodeEdit{render};
    nodeEdit.SetSettings(&settings);
    AddArgumentNodes(nodeEdit);
    AddConstantVarNodes(nodeEdit);
    AddMathNodes(nodeEdit);
    AddGlobalNodes(nodeEdit);
    AddRenderNodes(nodeEdit);
    AddSplitMergeNodes(nodeEdit);
    AddTransformNodes(nodeEdit);
    AddConditionalNodes(nodeEdit);

    {
        SereBridge::Context bridgeCtx;
        bridgeCtx.editor = &nodeEdit;
        bridgeCtx.render = &render;
        bridgeCtx.settings = &settings;
        unsigned short bridgePort = 8790;
        if (const char* env = getenv("SERE_BRIDGE_PORT"))
            bridgePort = (unsigned short)atoi(env);
        SereBridge::Start(bridgePort, bridgeCtx);
    }

    while (g_renderFramework->ShouldMainLoopRun())
    {

        // Handle window being minimized or screen locked
        if (!g_renderFramework->ImGuiStartFrame()) {
            continue;
        }

        // Start the Dear ImGui frame
        
        ImGui::NewFrame();

     
        ShowExampleAppDockSpace(&use_docking_space);
        
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    nodeEdit.New();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    nodeEdit.Load();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    nodeEdit.Save();
                }
                if (ImGui::MenuItem("Save As...")) {
                    nodeEdit.SaveAs();
                }
                if (ImGui::MenuItem("Export")) {
                    nodeEdit.Export();
                }
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("Settings")) {
                settings.Open();
            }
            
            ImGui::EndMainMenuBar();
        }
        settings.ShowSettingsWindow();
        nodeEdit.DrawUnsavedPrompt();
        nodeEdit.DrawExportPopup();
        if (settings.HasChanged()) {
            ReloadAssets(settings.GetTitanfall2Path());
            auto size = settings.GetRuiSize();
            render.SetSize(size.width,size.height);
            g_renderFramework->RuiReCreatePipeline(size.width,size.height);
        }
        

        SereBridge::Pump();

        render.StartFrame(ImGui::GetCurrentContext()->Time);
        nodeEdit.Draw();
        render.EndFrame();
        render.DrawImage();
        
       //ImPlot::ShowDemoWindow();
       // Rendering
       ImGui::Render();
       if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
       g_renderFramework->ImGuiEndFrame();
    }

    SereBridge::Stop();

    g_renderFramework->ImGuiDeInit();

    // Cleanup
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    
    return 0;
}


