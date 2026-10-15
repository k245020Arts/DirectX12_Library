#include "Application.h"
#include "../Window/Window.h"
#include "../Engine/Engine.h"
#include "../Object2D/Object2DManager.h"
#include "../Object3D/Object3DManager.h"
#include "../Time/DeltaTime.h"
#include "../Texture/TextureLoader.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"
#include "../Camera3D/Camera3D.h"
#include <vector>
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../../ImGui/imgui_impl_dx12.h"
#include "../Input/InputManager.h"

Application::Application()
{
	imgui = nullptr;
}

Application::~Application()
{

}

void Application::ApplicationInit(Window window)
{
	if (imgui == nullptr) {
		imgui = std::make_unique<DirectX12Imgui>();
	}
	imgui->SetUpImGui(window.GetHwnd());
	InputManager::GetInstance()->SetMouseHwnd(window.GetHwnd());
}

void Application::BeginUpdate()
{
	DeltaTime::GetInstance()->Update(); //デルタタイムの計測と固定FPSの制御

	if (imgui == nullptr) {
		return;
	}
	imgui->BeginRenderImGui();

	imgui->DebugRenderer();
}

void Application::ApplicationUpdate()
{
	Object2DManager::GetInstance()->Update(); //オブジェクト2Dの更新処理(スクリーン座標に置き換え等をしてる)
	Object3DManager::GetInstance()->Update(); //オブジェクト3Dの更新処理(カメラの設定ライトの設定等)
	InputManager::GetInstance()->Update();
	Engine::GetInstance()->BeginRender();	  //描画をする前の処理
}

void Application::ApplicationDraw()
{
	Object2DManager::GetInstance()->Draw(); //オブジェクト2Dの描画に必要な情報を流し込む
	Object3DManager::GetInstance()->Draw(); //オブジェクト3Dの描画に必要な情報を流し込む

	if (imgui != nullptr) {
		imgui->EndRenderImGui();
	}

	Engine::GetInstance()->EndRender(); //描画
}

void Application::ApplicationRelease()
{
	if (imgui != nullptr) {
		imgui->ReleaseImGui();
	}

	//破棄処理
	Engine::GetInstance()->Destroy();
	TextureLoader::GetInstance()->Destroy();
	Object2DManager::GetInstance()->Destroy();
	Object3DManager::GetInstance()->Destroy();
	DescriptorHeap::GetInstance()->Destroy();
	Camera3D::GetInstance()->Destroy();

}
