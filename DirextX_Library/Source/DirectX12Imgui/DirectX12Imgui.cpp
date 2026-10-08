#include "DirectX12Imgui.h"
#include "../Engine/Engine.h"
#include "../Time/DeltaTime.h"
#define IMGUI

DirectX12Imgui::DirectX12Imgui()
{
	
}

DirectX12Imgui::~DirectX12Imgui()
{
}

void DirectX12Imgui::SetUpImGui(HWND hwnd)
{
#ifdef IMGUI
	// ImGUI初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.Fonts->AddFontFromFileTTF(u8"c:\\Windows\\Fonts\\meiryo.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	imguiHandle = DescriptorHeap::GetInstance()->Allocate();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(Engine::GetInstance()->Device(), FRAME_BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, DescriptorHeap::GetInstance()->GetHeap().Get() , imguiHandle->handleCPU, imguiHandle->handleGPU);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 6.0f;
	style.FrameRounding = 4.0f;
	style.GrabRounding = 2.0f;
	style.ScrollbarRounding = 4.0f;
	style.WindowPadding = ImVec2(10.0f, 10.0f);
	style.ItemSpacing = ImVec2(10, 8);
	style.WindowBorderSize = 1.2f;
	style.FrameBorderSize = 1.0f;
	style.ChildRounding = 5.0f;
	style.PopupRounding = 5.0f;

	// ボタンなどの余白
	style.FramePadding = ImVec2(8.0f, 5.0f);
	style.ItemSpacing = ImVec2(8.0f, 6.0f);

	ImVec4* c = style.Colors;

	// 基本背景：黒に近い
	c[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.01f, 0.015f, 0.95f);
	c[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.01f, 0.015f, 0.90f);
	c[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.00f, 0.02f, 0.95f);
	c[ImGuiCol_Border] = ImVec4(0.70f, 0.00f, 0.10f, 0.45f);

	// テキスト（白・赤）
	c[ImGuiCol_Text] = ImVec4(1.00f, 0.90f, 0.9f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.30f, 0.35f, 1.00f);

	// ボタン：黒 → 濃赤 → 赤発光
	c[ImGuiCol_Button] = ImVec4(0.30f, 0.05f, 0.07f, 0.85f);
	c[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.10f, 0.15f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.20f, 0.25f, 1.00f);

	// 入力欄
	c[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.00f, 0.03f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.70f, 0.00f, 0.10f, 0.85f);
	c[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.10f, 0.20f, 1.00f);

	// スライダー、チェックマーク（赤紫系）
	c[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.10f, 0.40f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.30f, 0.60f, 1.00f);
	c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.15f, 0.35f, 1.00f);

	// タブ・タイトル
	c[ImGuiCol_TitleBg] = ImVec4(0.25f, 0.00f, 0.05f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.60f, 0.00f, 0.10f, 1.00f);
	c[ImGuiCol_Tab] = ImVec4(0.40f, 0.00f, 0.10f, 1.00f);
	c[ImGuiCol_TabHovered] = ImVec4(0.90f, 0.05f, 0.10f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(1.00f, 0.10f, 0.15f, 1.00f);

	// リサイズ/セパレータ
	c[ImGuiCol_Separator] = ImVec4(0.80f, 0.00f, 0.10f, 0.75f);
	c[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.00f, 0.10f, 0.40f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.10f, 0.20f, 0.85f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.30f, 0.35f, 1.00f);

	ImGuiIO& ioo = ImGui::GetIO();

	printf("ConfigFlags   : 0x%08X\n", ioo.ConfigFlags);
	printf("BackendFlags  : 0x%08X\n", ioo.BackendFlags);
	printf("PlatformName  : %s\n", ioo.BackendPlatformName);
	printf("RendererName  : %s\n", ioo.BackendRendererName);

	printf("PlatformHasViewports : %s\n",
		(ioo.BackendFlags & ImGuiBackendFlags_PlatformHasViewports)
		? "YES" : "NO");

	printf("RendererHasViewports : %s\n",
		(ioo.BackendFlags & ImGuiBackendFlags_RendererHasViewports)
		? "YES" : "NO");

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
