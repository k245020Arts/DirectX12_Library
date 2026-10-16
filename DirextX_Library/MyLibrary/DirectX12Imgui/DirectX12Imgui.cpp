#include "DirectX12Imgui.h"
#include "../Engine/Engine.h"
#include "../Time/DeltaTime.h"
#include "../Engine/Engine.h"

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

    // ImGui全体の文字サイズを80%にする
    io.FontGlobalScale = 1.0f;

    ImGuiStyle& style = ImGui::GetStyle();

    // ウィンドウ・ボタン・余白などを全体的に縮小
    style.ScaleAllSizes(0.5f);
}
