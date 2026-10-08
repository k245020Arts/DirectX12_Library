#include <string>
#include "Window/Window.h"
#include "Engine/Engine.h"
#include <assert.h>
#ifdef _DEBUG
#include <iostream>
#include <memory>
#endif // _DEBUG
#include "Scene/Scene.h"
#include "Object2D/Object2DManager.h"
#include "Time/DeltaTime.h"
#include "Texture/TextureLoader.h"
#include "DirectX12Imgui/DirectX12Imgui.h"

#include <vector>



#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../ImGui/imgui_impl_dx12.h"


class Main
{
public:
	
	static void Run();

private:
	
};

void DebugOutputFormatString(const char* format, ...) 
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif // _DEBUG

}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	Main::Run();
	return true;
}

//#define UseConsole

typedef Size ScreenSize;

void Main::Run()
{

#ifdef UseConsole
	if (AllocConsole()) { //コンソールウィンドウを表示
		FILE* fp = nullptr;
		freopen_s(&fp, "CONOUT$", "w", stdout);
	}
#endif

	DirectX12Imgui imgui;

	const int WINDOW_WINDTH = 1280;
	const int WINDOW_HEIGHT = 720;
	Window window;
	if (!window.Create(ScreenSize(WINDOW_WINDTH, WINDOW_HEIGHT), L"DX12_Library", L"Window")) {
		assert(false && "ウィンドウ作成失敗");
		return;
	}

	if (!Engine::GetInstance()->Init(window.GetHwnd(), window.GetWindowSize())) {
		return;
	}

	imgui.SetUpImGui(window.GetHwnd());

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}

		DeltaTime::GetInstance()->Update(); //デルタタイムの計測と固定FPSの制御

		imgui.BeginRenderImGui();

		imgui.DebugRenderer();

		scene->Update(); //シーンの更新
		Object2DManager::GetInstance()->Update(); //オブジェクト2Dの更新処理(スクリーン座標に置き換え等をしてる)
		Engine::GetInstance()->BeginRender(); //描画をする前の処理
		scene->Draw(); //描画に必要な情報を流し込む
		Object2DManager::GetInstance()->Draw(); //オブジェクト2Dの描画に必要な情報を流し込む

		imgui.EndRenderImGui();

		Engine::GetInstance()->EndRender(); //描画
	}

	imgui.ReleaseImGui();

	Engine::GetInstance()->Destroy();
	TextureLoader::GetInstance()->Destroy();
	Object2DManager::GetInstance()->Destroy();
	DescriptorHeap::GetInstance()->Destroy();
}
