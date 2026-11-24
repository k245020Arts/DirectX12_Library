#include "PostEffectCreater.h"
#include "../Window/Window.h"
#include "../Engine/Engine.h"
#include "../Object3D/Object3DManager.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"
#include "../RenderPass/RenderPass.h"
#include "../RenderPass/RenderPassManager.h"
#include "../PostEffect/PostEffectPipeline.h"
#include "../PostEffect/GaussianBlur/GaussianBlur.h"
#include "../PostEffect/Bloom/BloomEffect.h"
#include "../PostEffect/NormalMapPostEffect/DistortionPostEffect.h"
#include "../Shadow/ShadowMap.h"
#include "../Shadow/ShadowCamera.h"
#include "../Shadow/ShadowMapDebug.h"
#include "../Light/Light.h"
#include <algorithm>
#include <vector>
#include "../../ImGui/imgui.h"
#include "../Shadow/ShadowMapCreater.h"
#include "Fog/FogEffect.h"
#include "MotionBlur/MotionBlur.h"
#include "Vignette/Vignette.h"

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

   
    // フォグは深度(カメラからの距離)で色を変えるので、ぼかしや光を足す前の、いちばん最初に入れる

    fog = postEffectPipeline->AddEffect<FogEffect>();
    motionBlur = postEffectPipeline->AddEffect<MotionBlur>();

    // ブルームは最初に入れる(あとから足す被写界深度などは、ブルームより前に入れる)
    bloom = postEffectPipeline->AddEffect<BloomEffect>();
    gaussianBlur = postEffectPipeline->AddEffect<GaussianBlur>();
    distortion = postEffectPipeline->AddEffect<DistortionPostEffect>();
    vignette = postEffectPipeline->AddEffect<Vignette>();
   

    if (distortion != nullptr)
    {
        distortion->Load("data/testDist.png");
    }

    if (bloom != nullptr)
    {
        bloom->SetEnabled(false);
    }
    if (gaussianBlur != nullptr)
    {
        gaussianBlur->SetEnabled(false);
    }
    if (distortion != nullptr)
    {
        distortion->SetEnabled(false);
    }
    if (motionBlur != nullptr)
    {
        motionBlur->SetEnabled(false);
    }
    if (vignette != nullptr) {
        vignette->SetEnabled(false);
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

    if (vignette != nullptr)
    {
        bool enabled = vignette->IsEnabled();
        if (ImGui::Checkbox("Vignette", &enabled))
        {
            vignette->SetEnabled(enabled);
        }

        // 色（ColorEdit3でカラーピッカー表示）
        Vector3 color = vignette->GetVignetteColor();
        float colorArr[3] = { color.x, color.y, color.z };
        if (ImGui::ColorEdit3("Vignette Color", colorArr))
        {
            vignette->SetVignetteColor(colorArr[0], colorArr[1], colorArr[2]);
        }

        // 強さ
        float intensity = vignette->GetIntensity();
        if (ImGui::DragFloat("Vignette Intensity", &intensity, 0.01f, 0.0f, 3.0f, "%.2f"))
        {
            vignette->SetIntensity(intensity);
        }

        // 滑らかさ
        float smoothness = vignette->GetSmoothness();
        if (ImGui::DragFloat("Vignette Smoothness", &smoothness, 0.01f, 0.01f, 1.0f, "%.2f"))
        {
            vignette->SetSmoothness(smoothness);
        }

        // 真円度
        float roundness = vignette->GetRoundness();
        if (ImGui::DragFloat("Vignette Roundness", &roundness, 0.01f, 0.0f, 1.0f, "%.2f"))
        {
            vignette->SetRoundness(roundness);
        }

        // 中心位置（X, Yを2軸同時調整）
        Vector2 center = vignette->GetCenterPos();
        float centerArr[2] = { center.x, center.y };
        if (ImGui::DragFloat2("Vignette Center", centerArr, 0.01f, 0.0f, 1.0f, "%.2f"))
        {
            vignette->SetCenterPos(centerArr[0], centerArr[1]);
        }
    }


    if (motionBlur != nullptr)
    {
        bool enabled = motionBlur->IsEnabled();
        if (ImGui::Checkbox("MotionBlur", &enabled))
        {
            motionBlur->SetEnabled(enabled);
        }

        int count = motionBlur->GetSampleCount();
        if (ImGui::DragInt("MotionBlur Count", &count, 1, 0, 10, "%d"))
        {
            motionBlur->SetSampleCount(count);
        }

        float strength = motionBlur->GetStrength();
        if (ImGui::DragFloat("motionBlur Strength", &strength, 0.0005f, 0.0f, 1.0f, "%.4f"))
        {
            motionBlur->SetStrength(strength);
        }
    }

    if (fog != nullptr)
    {
        bool enabled = fog->IsEnabled();
        if (ImGui::Checkbox("Fog", &enabled))
        {
            fog->SetEnabled(enabled);
        }

        Vector4 color = fog->GetColor(color);
        if (ImGui::ColorEdit4("Fog Color", &color.x))
        {
            fog->SetColor(color);
        }

        float density = fog->GetDensity();
        if (ImGui::DragFloat("Fog Density", &density, 0.0005f, 0.0f, 1.0f, "%.4f"))
        {
            fog->SetDensity(density);
        }

        float falloff = fog->GetHeightFalloff();
        if (ImGui::DragFloat("Fog Height Falloff", &falloff, 0.001f, 0.0f, 2.0f, "%.4f"))
        {
            fog->SetHeightFalloff(falloff);
        }

        float baseHeight = fog->GetBaseHeight();
        if (ImGui::DragFloat("Fog Base Height", &baseHeight, 0.5f, -1000.0f, 1000.0f))
        {
            fog->SetBaseHeight(baseHeight);
        }

        float startDistance = fog->GetStartDistance();
        if (ImGui::DragFloat("Fog Start Distance", &startDistance, 1.0f, 0.0f, 10000.0f))
        {
            fog->SetStartDistance(startDistance);
        }

        float maxOpacity = fog->GetMaxOpacity();
        if (ImGui::SliderFloat("Fog Max Opacity", &maxOpacity, 0.0f, 1.0f))
        {
            fog->SetMaxOpacity(maxOpacity);
        }

        float sky = fog->GetSkyAmount();
        if (ImGui::SliderFloat("Fog Sky", &sky, 0.0f, 1.0f))
        {
            fog->SetSkyAmount(sky);
        }

        ImGui::Separator();
    }

    if (bloom != nullptr)
    {
        bool enabled = bloom->IsEnabled();
        if (ImGui::Checkbox("Bloom", &enabled))
        {
            bloom->SetEnabled(enabled);
        }

        float threshold = bloom->GetThreshold();
        if (ImGui::SliderFloat("Bloom Threshold", &threshold, 0.0f, 0.99f))
        {
            bloom->SetThreshold(threshold);
        }

        float intensity = bloom->GetIntensity();
        if (ImGui::SliderFloat("Bloom Intensity", &intensity, 0.0f, 3.0f))
        {
            bloom->SetIntensity(intensity);
        }

        float bloomSigma = bloom->GetSigma();
        if (ImGui::SliderFloat("Bloom Sigma", &bloomSigma, 0.1f, 10.0f))
        {
            bloom->SetSigma(bloomSigma);
        }

        float bloomSpread = bloom->GetSpread();
        if (ImGui::SliderFloat("Bloom Spread", &bloomSpread, 0.0f, 8.0f))
        {
            bloom->SetSpread(bloomSpread);
        }
    }

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

void PostEffectCreater::EnableBloom(bool _enable)
{
    if (bloom != nullptr)
    {
        bloom->SetEnabled(_enable);
    }
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
