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

//#define UseConsole;

void Main::Run()
{

#ifdef UseConsole
	if (AllocConsole()) {
		FILE* fp = nullptr;
		freopen_s(&fp, "CONOUT$", "w", stdout);
	}
#endif

	Window window;
	if (!window.Create(Size(1280, 720), L"DX12_Library", L"Window")) {
		assert(false && "ウィンドウ作成失敗");
		return;
	}

	if (!Engine::GetInstance()->Init(window.GetHwnd(), window.GetWindowSize())) {
		return;
	}

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}

		DeltaTime::GetInstance()->Update();
		scene->Update();
		Object2DManager::GetInstance()->Update();
		Engine::GetInstance()->BeginRender();
		scene->Draw();
		Object2DManager::GetInstance()->Draw();
		Engine::GetInstance()->EndRender();

		std::string fps = std::to_string(DeltaTime::GetInstance()->GetFPS());
		printf("%s \n", fps.c_str());
	}

	Engine::GetInstance()->Destroy();
	Object2DManager::GetInstance()->Destroy();
}
