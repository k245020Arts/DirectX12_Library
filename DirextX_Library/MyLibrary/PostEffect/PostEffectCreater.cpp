#include "PostEffectCreater.h"
#include "../Window/Window.h"
#include "../Engine/Engine.h"
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
#include "../RenderPass/RenderPass.h"
#include "../RenderPass/RenderPassManager.h"
#include "../RenderTarget/RenderTarget.h"
#include "../PostEffect/PostEffectPipeline.h"
#include "../PostEffect/GaussianBlur/GaussianBlur.h"
#include "../PostEffect/NormalMapPostEffect/DistortionPostEffect.h"
#include "../Shadow/ShadowMap.h"


PostEffectCreater::PostEffectCreater()
{

}

PostEffectCreater::~PostEffectCreater()
{

}

void PostEffectCreater::Init()
{
	auto size = Engine::GetInstance()->GetWindowSize();

	// ShadowMap生成
	/*shadowMap = std::make_shared<ShadowMap>();

	if (!shadowMap->Create(2048, 2048))
	{
		OutputDebugStringW(L"ShadowMapの生成に失敗\n");
		return;
	}*/

	// ポストエフェクト
	// RenderTarget / RenderPass / 入力テクスチャの設定は Build() が自動でやる
	postEffectPipeline = std::make_unique<PostEffectPipeline>();

	postEffectPipeline->Init(size.width, size.height);

	// シーン描画
	postEffectPipeline->SetSceneFunction([]()
	{
		Object3DManager::GetInstance()->Draw();
	});

	/*auto shadowPass = std::make_shared<RenderPass>();

	shadowPass->SetExecuteFunction([this]()
	{
		shadowMap->BeginRender();

		Object3DManager::GetInstance()->DrawShadow(*shadowMap.get());

		shadowMap->EndRender();
	});*/

	// RenderPassManagerに登録
	/*RenderPassManager::GetInstance()->AddPass(shadowPass);*/

	// 使いたいエフェクトを、かけたい順に追加するだけ
	// 返ってきたポインタで、パラメータの変更や SetEnabled(true/false) による個別のON/OFFができる
	gaussianBlur = postEffectPipeline->AddEffect<GaussianBlur>();

	distortion = postEffectPipeline->AddEffect<DistortionPostEffect>();

	if (distortion != nullptr)
	{
		distortion->Load("data/testDist.png");
	}

	// 最初は全部OFF (何も有効でなければ BackBuffer へ直接描画)
	if (gaussianBlur != nullptr)
	{
		gaussianBlur->SetEnabled(false);
	}

	if (distortion != nullptr)
	{
		distortion->SetEnabled(false);
	}

	// RenderTarget と RenderPass を構築して RenderPassManager へ登録
	postEffectPipeline->Build();
}

void PostEffectCreater::Update()
{
#ifdef IMGUI
	// ポストエフェクトの操作パネル (ON/OFF とパラメータ)
	if (gaussianBlur != nullptr || distortion != nullptr)
	{
		ImGui::Begin("PostEffect");

		if (gaussianBlur != nullptr)
		{
			bool enabled = gaussianBlur->IsEnabled();

			if (ImGui::Checkbox("GaussianBlur", &enabled))
			{
				gaussianBlur->SetEnabled(enabled);
			}

			float sigma = gaussianBlur->GetSigma();

			if (ImGui::SliderFloat("Blur Sigma", &sigma, 0.1f, 10.0f))
			{
				gaussianBlur->SetSigma(sigma);
			}

			float spread = gaussianBlur->GetSpread();

			if (ImGui::SliderFloat("Blur Spread", &spread, 0.0f, 4.0f))
			{
				gaussianBlur->SetSpread(spread);
			}
		}

		if (distortion != nullptr)
		{
			bool enabled = distortion->IsEnabled();

			if (ImGui::Checkbox("Distortion", &enabled))
			{
				distortion->SetEnabled(enabled);
			}

			float strength = distortion->GetStrength();

			if (ImGui::SliderFloat("Distortion Strength", &strength, 0.0f, 0.5f))
			{
				distortion->SetStrength(strength);
			}
		}

		ImGui::End();
	}
#endif // IMGUI
}

void PostEffectCreater::EnableGaussainBlur(bool _enable)
{
	gaussianBlur->SetEnabled(_enable);
}

void PostEffectCreater::EnableDistortion(bool _enable)
{
	distortion->SetEnabled(_enable);
}

