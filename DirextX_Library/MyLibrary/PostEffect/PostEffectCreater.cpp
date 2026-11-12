#include "PostEffectCreater.h"
#include "../Window/Window.h"
#include "../Engine/Engine.h"
#include "../Object3D/Object3DManager.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"
#include "../RenderPass/RenderPass.h"
#include "../RenderPass/RenderPassManager.h"
#include "../PostEffect/PostEffectPipeline.h"
#include "../PostEffect/GaussianBlur/GaussianBlur.h"
#include "../PostEffect/NormalMapPostEffect/DistortionPostEffect.h"
#include "../Shadow/ShadowMap.h"
#include "../Shadow/ShadowCamera.h"
#include "../Shadow/ShadowMapDebug.h"
#include "../Light/Light.h"
#include <algorithm>
#include <vector>
#include "../../ImGui/imgui.h"
#include "../Shadow/ShadowMapCreater.h"

PostEffectCreater::PostEffectCreater()
{
}

PostEffectCreater::~PostEffectCreater()
{
}

void PostEffectCreater::Init()
{
    const auto size = Engine::GetInstance()->GetWindowSize();

    // ---------------------------------------------------------------------
    // 1) ShadowMapとライトカメラを作り、シーン描画より前のRenderPassに登録する。
    // ---------------------------------------------------------------------
    ShadowMapCreater::GetInstance()->Init();

    // ---------------------------------------------------------------------
    // 2) 既存のポストエフェクトチェーンを構築する。
    // ---------------------------------------------------------------------
    postEffectPipeline = std::make_unique<PostEffectPipeline>();
    if (!postEffectPipeline->Init(size.width, size.height))
    {
        OutputDebugStringW(L"PostEffectPipelineの初期化に失敗\n");
        return;
    }

    postEffectPipeline->SetSceneFunction([]()
    {
        Object3DManager::GetInstance()->Draw();
    });

    gaussianBlur = postEffectPipeline->AddEffect<GaussianBlur>();
    distortion = postEffectPipeline->AddEffect<DistortionPostEffect>();

    if (distortion != nullptr)
    {
        distortion->Load("data/testDist.png");
    }

    if (gaussianBlur != nullptr)
    {
        gaussianBlur->SetEnabled(false);
    }
    if (distortion != nullptr)
    {
        distortion->SetEnabled(false);
    }

    // Scene -> PostEffectの順で登録される。ShadowPassはその前に登録済み。
    if (!postEffectPipeline->Build())
    {
        OutputDebugStringW(L"PostEffectPipelineのBuildに失敗\n");
        return;
    }

    // ---------------------------------------------------------------------
    // 3) 深度Debugは全ポストエフェクトの後でBackBufferへ描画する。
    // ---------------------------------------------------------------------
    ShadowMapCreater::GetInstance()->DebugDrawInit();
}

void PostEffectCreater::Update()
{
#ifdef IMGUI
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

    ShadowMapCreater::GetInstance()->Update();

    ImGui::End();
#endif // IMGUI
}

void PostEffectCreater::EnableGaussainBlur(bool _enable)
{
    if (gaussianBlur != nullptr)
    {
        gaussianBlur->SetEnabled(_enable);
    }
}

void PostEffectCreater::EnableDistortion(bool _enable)
{
    if (distortion != nullptr)
    {
        distortion->SetEnabled(_enable);
    }
}
