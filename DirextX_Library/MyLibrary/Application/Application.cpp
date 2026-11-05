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
#include "../../KLibrary/File/FileExplorer/FileExplorer.h"
#include "../RenderPass/RenderPass.h"
#include "../RenderPass/RenderPassManager.h"
#include "../RenderTarget/RenderTarget.h"
#include "../PostEffect/PostEffect.h"
#include "../PostEffect/GaussianBlur/GaussianBlur.h"
#include "../PostEffect/NormalMapPostEffect/DistortionPostEffect.h"

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

#ifdef IMGUI
	FileExplorer::Init();
#endif // IMGUI

    auto size = Engine::GetInstance()->GetWindowSize();

    // シーン描画用RenderTarget
    sceneRenderTarget =std::make_shared<RenderTarget>();

    sceneRenderTarget->Create( size.width, size.height, DXGI_FORMAT_R8G8B8A8_UNORM );

	blurRenderTarget = std::make_shared<RenderTarget>();
	blurRenderTarget->Create(size.width,size.height,DXGI_FORMAT_R8G8B8A8_UNORM);

    // ポストエフェクト
	gaussianBlur = std::make_unique<GaussianBlur>();

	gaussianBlur->Init();

	gaussianBlur->SetInputTexture(sceneRenderTarget->GetSRV());

    // ScenePass
    scenePass = std::make_shared<RenderPass>();

    scenePass->SetRenderTarget(sceneRenderTarget.get());

    scenePass->SetExecuteFunction([]()
    {
        Object3DManager::GetInstance()->Draw();
    });

	// 水平方向のブラー
	horizontalBlurPass = std::make_shared<RenderPass>();

	horizontalBlurPass->SetRenderTarget(gaussianBlur->GetIntermediateRenderTarget());

	horizontalBlurPass->SetExecuteFunction([this]()
	{
		gaussianBlur->DrawHorizontal();
	});


	// 垂直方向のブラー
	verticalBlurPass = std::make_shared<RenderPass>();

	// nullptr = BackBuffer
	verticalBlurPass->SetRenderTarget(nullptr);

	verticalBlurPass->SetExecuteFunction([this]()
	{
		gaussianBlur->DrawVertical();
	});

	// 歪み
	distortion = std::make_unique<DistortionPostEffect>();

	distortion->Load("data/testDist.png");
	distortion->Init();

	distortion->SetInputTexture(blurRenderTarget->GetSRV());

	// 歪みPass
	distortionPass = std::make_shared<RenderPass>();

	// nullptr = BackBuffer
	distortionPass->SetRenderTarget(nullptr);

	distortionPass->SetExecuteFunction([this]()
	{
		distortion->Draw();
	});


    // 登録
    RenderPassManager::GetInstance()->AddPass(scenePass);

    RenderPassManager::GetInstance()->AddPass(horizontalBlurPass);
    RenderPassManager::GetInstance()->AddPass(verticalBlurPass);
    //RenderPassManager::GetInstance()->AddPass(distortionPass);
}

void Application::BeginUpdate()
{
	DeltaTime::GetInstance()->Update(); //デルタタイムの計測と固定FPSの制御

	if (imgui == nullptr) {
		return;
	}
	imgui->BeginRenderImGui();

	imgui->DebugRenderer();
#ifdef IMGUI
	FileExplorer::Update();
#endif // IMGUI

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
	//Object3DManager::GetInstance()->Draw(); //オブジェクト3Dの描画に必要な情報を流し込む
	// マルチパスレンダリング
	RenderPassManager::GetInstance()->Execute();

	// UI描画用にBackBufferを再設定
	// PostEffectPassがBackBufferを使用しているため
	Object2DManager::GetInstance()->Draw(); //オブジェクト2Dの描画に必要な情報を流し込む

	if (imgui != nullptr) {
		imgui->EndRenderImGui();
	}

	Engine::GetInstance()->EndRender(); //描画
}

void Application::ApplicationRelease()
{
	FileExplorer::Release();

	if (imgui != nullptr) {
		imgui->ReleaseImGui();
	}
	sceneRenderTarget.reset();

	scenePass.reset();
	verticalBlurPass.reset();
	horizontalBlurPass.reset();
	distortion.reset();

	gaussianBlur.reset();
	blurRenderTarget.reset();
	//破棄処理
	RenderPassManager::GetInstance()->Destroy();
	Engine::GetInstance()->Destroy();
	TextureLoader::GetInstance()->Destroy();
	Object2DManager::GetInstance()->Destroy();
	Object3DManager::GetInstance()->Destroy();
	DescriptorHeap::GetInstance()->Destroy();
	Camera3D::GetInstance()->Destroy();

}
