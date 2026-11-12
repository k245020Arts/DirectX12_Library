#include "ShadowMapCreater.h"
#include "ShadowMap.h"
#include "ShadowCamera.h"
#include "ShadowMapDebug.h"
#include "../RenderPass/RenderPassManager.h"
#include "../Light/Light.h"
#include "../Object3D/Object3DManager.h"
#include "../../ImGui/imgui.h"

ShadowMapCreater::ShadowMapCreater()
{
}

ShadowMapCreater::~ShadowMapCreater()
{
}

void ShadowMapCreater::Init()
{

    shadowMap = std::make_unique<ShadowMap>();
    shadowCamera = std::make_unique<ShadowCamera>();

    debugShadow = std::make_unique<ShadowMapDebug>();

    shadowMapCreated = shadowMap->Create(2048, 2048);

    if (shadowMapCreated)
    {
        shadowRenderPass = std::make_shared<RenderPass>();
        shadowRenderPass->SetBeginFunction([this]()
        {
            shadowMap->BeginRender();
        });

        shadowRenderPass->SetExecuteFunction([this]()
         {
             shadowCamera->Update();
             Object3DManager::GetInstance()->DrawShadow(*shadowCamera, *shadowMap.get());
         });

        shadowRenderPass->SetEndFunction([this]()
        {
             shadowMap->EndRender();
        });

        // 必ずSceneRenderPassより先に登録する。
        RenderPassManager::GetInstance()->AddPass(shadowRenderPass);
        shadowRenderPass->SetEnabled(useShadowMap);
    }
    else
    {
        OutputDebugStringW(L"ShadowMapの初期化に失敗したため、ShadowPassを無効にします\n");
        debugShadow.reset();
        shadowCamera.reset();
        shadowMap.reset();
    }

    const auto& lightData = Light::GetInstance()->GetLightBufferData();
    if (lightData.lightCount != 0)
    {
        //一番先頭のライトの方向をデフォルト値として設定
        shadowCamera->SetDirection(lightData.lights[0].direction);
    }
    else {
        shadowCamera->SetDirection(Vector3(0.f,1.f,0.f));
    }

    
}

void ShadowMapCreater::DebugDrawInit()
{
    const bool shadowDebugCreated = shadowMapCreated && debugShadow->Init();


    if (shadowMapCreated && !shadowDebugCreated)
    {
        OutputDebugStringW(L"ShadowMapDebugの初期化に失敗しました。深度生成のみ有効です\n");
        debugShadow.reset();
    }

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

void ShadowMapCreater::ShadowCameraDirection(const Vector3& _direction)
{
    if (shadowCamera == nullptr)
        return;
    shadowCamera->SetDirection(_direction);
}

void ShadowMapCreater::SetUseShadowMap(const bool _use, const int _w, const int _h)
{
    if (shadowMap == nullptr)
        return;

    shadowRenderPass->SetEnabled(_use);
    useShadowMap = _use;
    if (!_use)
        return;

    bool wCheck = _w % 2 == 0;
    bool hCheck = _h % 2 == 0;

    assert(wCheck && "wは2の倍数で指定してください");
    assert(hCheck && "hは2の倍数で指定してください");

    shadowMap->SetSize(_w, _h);
}

void ShadowMapCreater::Update()
{
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
}