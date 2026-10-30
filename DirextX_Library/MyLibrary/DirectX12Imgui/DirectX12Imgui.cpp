#include "DirectX12Imgui.h"
#include "../Engine/Engine.h"
#include "../Time/DeltaTime.h"
#include "../Engine/Engine.h"



DirectX12Imgui::DirectX12Imgui()
{
	
}

DirectX12Imgui::~DirectX12Imgui()
{
}

void DirectX12Imgui::SetUpImGui(HWND hwnd)
{
#ifdef IMGUI

    //ImGui_ImplWin32_EnableDpiAwareness();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // 2. フォントの設定
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\meiryo.ttc",
        16.0f,
        nullptr,
        io.Fonts->GetGlyphRangesJapanese()
    );

    IM_ASSERT(font != nullptr);

    // 3. スタイル設定
    SetUpStyle();

    // 4. Win32 バックエンドの初期化
    IM_ASSERT(ImGui_ImplWin32_Init(hwnd));

    ImGui_ImplDX12_InitInfo initInfo{};

    initInfo.Device =
        Engine::GetInstance()->Device();

    initInfo.CommandQueue =
        Engine::GetInstance()->CommandQueue();

    initInfo.NumFramesInFlight = FRAME_BUFFER_COUNT;
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;

    initInfo.SrvDescriptorHeap =
        DescriptorHeap::GetInstance()->GetHeap().Get();

    initInfo.SrvDescriptorAllocFn =
        [](ImGui_ImplDX12_InitInfo*,
            D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
            D3D12_GPU_DESCRIPTOR_HANDLE* gpu)
        {
            auto* handle = DescriptorHeap::GetInstance()->Allocate();

            assert(handle != nullptr);

            if (handle)
            {
                *cpu = handle->handleCPU;
                *gpu = handle->handleGPU;
            }
        };

    initInfo.SrvDescriptorFreeFn =[](ImGui_ImplDX12_InitInfo*,
            D3D12_CPU_DESCRIPTOR_HANDLE cpu,
            D3D12_GPU_DESCRIPTOR_HANDLE gpu)
        {
            DescriptorHeap::GetInstance()->Free(cpu, gpu);
        };

    IM_ASSERT(ImGui_ImplDX12_Init(&initInfo));

#endif
}

void DirectX12Imgui::BeginRenderImGui()
{

#ifdef IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

}

void DirectX12Imgui::EndRenderImGui()
{
#ifdef IMGUI
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Engine::GetInstance()->CommandList());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
}

void DirectX12Imgui::ReleaseImGui()
{
#ifdef IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete imguiHandle;
#endif
}

void DirectX12Imgui::DebugRenderer()
{
#ifdef IMGUI

#ifdef  _DEBUG

	ImGui::Begin("Debug");

	std::string fps = std::to_string(DeltaTime::GetInstance()->GetFPS());
	ImGui::Text("FPS = %s \n", fps.c_str());

	ImGui::End();

#endif //  _DEBUG

#endif // IMGUI
}

void DirectX12Imgui::SetUpStyle()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::StyleColorsDark();

    //---------- 形状：カクカク＋線くっきり----------
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;   // ボタン・入力欄にも枠線を出す
    style.TabBorderSize = 1.0f;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 5);
    style.ItemSpacing = ImVec2(8, 7);
    style.ItemInnerSpacing = ImVec2(6, 5);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    // ---------- カラー ----------
    auto RGBA = [](int r, int g, int b, int a) {
        return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
        };

    //シアン
    const ImVec4 primary = RGBA(0x1c, 0xfe, 0xff, 0xcc);
    const ImVec4 primaryA99 = RGBA(0x1c, 0xfe, 0xff, 0x99);
    const ImVec4 primaryA66 = RGBA(0x1c, 0xfe, 0xff, 0x66);
    const ImVec4 primaryA33 = RGBA(0x1c, 0xfe, 0xff, 0x33);
    const ImVec4 primaryDark = RGBA(0x03, 0x19, 0x1a, 0xee); // 暗い青緑
    const ImVec4 primaryDeep = RGBA(0x02, 0x10, 0x11, 0xff);

    //オレンジ
    const ImVec4 accent = RGBA(0xff, 0xa1, 0x00, 0xee);
    const ImVec4 accentHov = RGBA(0xff, 0xa1, 0x00, 0xcc);
    const ImVec4 accentA99 = RGBA(0xff, 0xa1, 0x00, 0x99);

    const ImVec4 blackA55 = RGBA(0x11, 0x11, 0x11, 0x55);

    ImVec4* c = style.Colors;

    // 背景
    c[ImGuiCol_WindowBg] = primaryDark;
    c[ImGuiCol_ChildBg] = RGBA(0, 0, 0, 0x30);
    c[ImGuiCol_PopupBg] = primaryDark;
    c[ImGuiCol_Border] = primaryA99;
    c[ImGuiCol_BorderShadow] = RGBA(0, 0, 0, 0);

    // タイトルバー・メニューバー
    c[ImGuiCol_TitleBg] = primaryDark;
    c[ImGuiCol_TitleBgActive] = primaryA99;
    c[ImGuiCol_TitleBgCollapsed] = primaryDark;
    c[ImGuiCol_MenuBarBg] = primaryA33;

    // スクロールバー
    c[ImGuiCol_ScrollbarBg] = primaryA33;
    c[ImGuiCol_ScrollbarGrab] = primaryA99;
    c[ImGuiCol_ScrollbarGrabHovered] = primaryA99;
    c[ImGuiCol_ScrollbarGrabActive] = primary;

    // チェック・スライダー
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = primaryA99;
    c[ImGuiCol_SliderGrabActive] = primary;

    // 区切り線（記事の Column 系の代替）
    c[ImGuiCol_Separator] = blackA55;
    c[ImGuiCol_SeparatorHovered] = accentHov;
    c[ImGuiCol_SeparatorActive] = accent;

    c[ImGuiCol_ResizeGrip] = primaryA33;
    c[ImGuiCol_ResizeGripHovered] = accentHov;
    c[ImGuiCol_ResizeGripActive] = accent;

    // タブ（v1.93 の名前）
    c[ImGuiCol_Tab] = primaryA33;
    c[ImGuiCol_TabHovered] = accentHov;
    c[ImGuiCol_TabSelected] = primaryA99;
    c[ImGuiCol_TabSelectedOverline] = accent;
    c[ImGuiCol_TabDimmed] = primaryDark;
    c[ImGuiCol_TabDimmedSelected] = primaryA66;
    c[ImGuiCol_TabDimmedSelectedOverline] = primaryA99;

    // ドッキング
    c[ImGuiCol_DockingPreview] = accentA99;
    c[ImGuiCol_DockingEmptyBg] = primaryDeep;

    // グラフ・テーブル
    c[ImGuiCol_PlotLines] = primaryA99;
    c[ImGuiCol_PlotLinesHovered] = primary;
    c[ImGuiCol_PlotHistogram] = primaryA99;
    c[ImGuiCol_PlotHistogramHovered] = primary;
    c[ImGuiCol_TableHeaderBg] = primaryA33;
    c[ImGuiCol_TableBorderStrong] = primaryA99;
    c[ImGuiCol_TableBorderLight] = primaryA33;
    c[ImGuiCol_TableRowBg] = RGBA(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = RGBA(0x1c, 0xfe, 0xff, 0x0a);

    // その他
    c[ImGuiCol_DragDropTarget] = accent;
    c[ImGuiCol_NavCursor] = accent;
    c[ImGuiCol_NavWindowingHighlight] = primary;
    c[ImGuiCol_NavWindowingDimBg] = RGBA(0x00, 0x20, 0x20, 0x50);
    c[ImGuiCol_ModalWindowDimBg] = RGBA(0x00, 0x10, 0x10, 0x90);

    const ImVec4 textMain = RGBA(0xE6, 0xFF, 0xFF, 0xFF);
    const ImVec4 textDisabled = RGBA(0x7F, 0xB5, 0xB8, 0xFF);

    // hover / active は「暗めのオレンジ」にして、白い文字のコントラストを確保
    const ImVec4 accentHovDk = RGBA(0xB8, 0x6A, 0x00, 0xF0);
    const ImVec4 accentActDk = RGBA(0xE0, 0x84, 0x00, 0xFF);
    const ImVec4 accentHdrDk = RGBA(0xB8, 0x6A, 0x00, 0x90);

    c[ImGuiCol_Text] = textMain;
    c[ImGuiCol_TextDisabled] = textDisabled;
    c[ImGuiCol_TextSelectedBg] = RGBA(0xFF, 0xA1, 0x00, 0x70); // 半透明にして文字を潰さない

    // 入力欄
    c[ImGuiCol_FrameBg] = RGBA(0x1c, 0xfe, 0xff, 0x22);
    c[ImGuiCol_FrameBgHovered] = accentHovDk;
    c[ImGuiCol_FrameBgActive] = accentActDk;

    // ボタン
    c[ImGuiCol_Button] = RGBA(0x1c, 0xfe, 0xff, 0x2a);
    c[ImGuiCol_ButtonHovered] = accentHovDk;
    c[ImGuiCol_ButtonActive] = accentActDk;

    // ヘッダー（TreeNode / Selectable）
    c[ImGuiCol_Header] = accentHdrDk;
    c[ImGuiCol_HeaderHovered] = accentHovDk;
    c[ImGuiCol_HeaderActive] = accentActDk;

    // タブ
    c[ImGuiCol_TabHovered] = accentHovDk;

    // ---------- ビューポート有効時の注意 ----------
    // 別OSウィンドウになった時に半透明だと背後が透けるので不透明化
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        c[ImGuiCol_WindowBg].w = 1.0f;
        c[ImGuiCol_PopupBg].w = 1.0f;
    }
}