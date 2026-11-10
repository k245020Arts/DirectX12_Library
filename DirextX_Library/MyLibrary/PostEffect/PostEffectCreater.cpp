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
    shadowMap = std::make_unique<ShadowMap>();
    shadowCamera = std::make_unique<ShadowCamera>();
    debugShadow = std::make_unique<ShadowMapDebug>();

    const bool shadowMapCreated = shadowMap->Create(4098, 4098);
    const bool shadowDebugCreated = shadowMapCreated && debugShadow->Init();

    if (shadowMapCreated)
    {
        shadowRenderPass = std::make_shared<RenderPass>();
        shadowRenderPass->SetBeginFunction([this]()
        {
            shadowMap->BeginRender();
        });

        shadowRenderPass->SetExecuteFunction([this]()
        {
            const auto& lightData = Light::GetInstance()->GetLightBufferData();
            if (lightData.lightCount == 0)
            {
                return;
            }

            // 現在の先頭ライトをディレクショナルライトとして使用する。
            shadowCamera->Update(lightData.lights[0].direction);
            Object3DManager::GetInstance()->DrawShadow(*shadowCamera,*shadowMap.get());
        });

        shadowRenderPass->SetEndFunction([this]()
        {
            shadowMap->EndRender();
        });

        // 必ずSceneRenderPassより先に登録する。
        RenderPassManager::GetInstance()->AddPass(shadowRenderPass);
    }
    else
    {
        OutputDebugStringW(L"ShadowMapの初期化に失敗したため、ShadowPassを無効にします\n");
        debugShadow.reset();
        shadowCamera.reset();
        shadowMap.reset();
    }

    if (shadowMapCreated && !shadowDebugCreated)
    {
        OutputDebugStringW(L"ShadowMapDebugの初期化に失敗しました。深度生成のみ有効です\n");
        debugShadow.reset();
    }

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
    if (shadowDebugCreated)
    {
        debugShadowPass = std::make_shared<RenderPass>();
        debugShadowPass->SetExecuteFunction([this]()
        {
            if (shadowMap != nullptr && debugShadow != nullptr)
            {
                debugShadow->Draw(*shadowMap);
            }
        });
        debugShadowPass->SetEnabled(false);
        RenderPassManager::GetInstance()->AddPass(debugShadowPass);
    }
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

    if (debugShadowPass != nullptr)
    {
        if (ImGui::Checkbox("Show ShadowMap Depth", &showShadowMap))
        {
            debugShadowPass->SetEnabled(showShadowMap);
        }

        if (shadowCamera != nullptr)
        {
            const auto& currentTarget = shadowCamera->GetTarget();
            float target[3] = { currentTarget.x, currentTarget.y, currentTarget.z };
            if (ImGui::DragFloat3("Shadow Target", target, 1.0f))
            {
                shadowCamera->SetTarget(Vector3(target[0], target[1], target[2]));
            }

            float orthoWidth = shadowCamera->GetOrthographicWidth();
            float orthoHeight = shadowCamera->GetOrthographicHeight();
            if (ImGui::SliderFloat("Shadow Ortho Width", &orthoWidth, 100.0f, 3000.0f))
            {
                shadowCamera->SetOrthographicSize(orthoWidth, orthoHeight);
            }
            if (ImGui::SliderFloat("Shadow Ortho Height", &orthoHeight, 100.0f, 3000.0f))
            {
                shadowCamera->SetOrthographicSize(orthoWidth, orthoHeight);
            }
        }
    }

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
