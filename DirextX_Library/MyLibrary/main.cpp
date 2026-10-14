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

#include "Application/Application.h"

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

	const int WINDOW_WINDTH = 1280;
	const int WINDOW_HEIGHT = 720;
	Window window;
	if (!window.Create(ScreenSize(WINDOW_WINDTH, WINDOW_HEIGHT), L"DX12_Library", L"Window")) {
		assert(false && "ウィンドウ作成失敗");
		return;
	}

	if (!Engine::GetInstance()->Init(window.GetHwnd(), window.GetWindowSize())) {
		assert(false && "初期化ミス");
		return;
	}

	Application::GetInstance()->ApplicationInit(window);

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}

		Application::GetInstance()->BeginUpdate(); //SceneのUpdateの前にしないといけない処理

		scene->Update(); //シーンの更新
		
		Application::GetInstance()->ApplicationUpdate(); //SceneのDrawの前にしないといけない処理

		scene->Draw(); //描画に必要な情報を流し込む

		Application::GetInstance()->ApplicationDraw(); // 最終的な描画
	}

	Application::GetInstance()->ApplicationRelease(); //内部で回しているものの破棄処理
	Application::Destroy(); //最後にアプリケーションの破棄
}
