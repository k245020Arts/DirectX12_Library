#include <string>
#include "Window/Window.h"
#include "Engine/Engine.h"
#include <assert.h>
#ifdef _DEBUG
#include <iostream>
#include <memory>
#endif // _DEBUG


#include <vector>

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
	
	Window window;
	if (!window.Create(Size(1280,720), L"DX12_Library", L"Window")) { //初期化
		assert(false && "ウィンドウ作成失敗");
		return 0;
	}

	// 描画エンジンの初期化を行う
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();
	if (!engine->Init(window.GetHwnd(), window.GetWindowSize()))
	{
		return -1;
	}
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}

		//ToDO 後でこの行で更新処理を行う
		engine->BeginRender();

		//ToDO 後でこの行で3Dオブジェクトのの描画処理を行う

		engine->EndRender();
	}
	return 1;
}

