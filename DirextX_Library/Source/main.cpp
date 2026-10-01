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

#include <vector>

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

void Main::Run()
{
	Window window;
	if (!window.Create(Size(1280, 720), L"DX12_Library", L"Window")) { //初期化
		assert(false && "ウィンドウ作成失敗");
		return ;
	}

	// 描画エンジンの初期化を行う
	if (!Engine::GetInstance()->Init(window.GetHwnd(), window.GetWindowSize()))
	{
		return;
	}
	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}

		scene->Update(); //シーンでの更新処理
		Object2DManager::GetInstance()->Update(); //2DObjectの座標の位置更新処理(ここでスクリーン座標にしている)
		Engine::GetInstance()->BeginRender(); //描画の準備
		scene->Draw(); //必要ならデータを流す
		Object2DManager::GetInstance()->Draw(); //2DObjectの描画の情報をコマンドリストに流し込む
		Engine::GetInstance()->EndRender(); //流した情報を画面に描画させる

	}

	Engine::GetInstance()->Destroy();
	Object2DManager::GetInstance()->Destroy();
}
