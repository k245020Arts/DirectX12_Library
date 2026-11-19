#include "FogEffect.h"
#include <algorithm>
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../Camera3D/Camera3D.h"

FogEffect::FogEffect()
{
}

FogEffect::~FogEffect()
{
}

bool FogEffect::Init()
{
    if (!param.Create())
    {
        return false;
    }

    // ルートシグネチャ : t0(色), t1(深度), b0 (ブルームと同じ2入力のもの)
    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectTwoInputRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/Fog/FogPixelShader.hlsl", "FogPS");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"FogEffect : パイプラインステートの生成に失敗\n");
        return false;
    }

    return true;
}

void FogEffect::Draw()
{
    DrawFog();
}

std::vector<PostEffectPassInfo> FogEffect::CreatePasses()
{
    // 1パスで完結する最終出力パスのみ
    return { { [this]() { DrawFog(); }, nullptr } };
}

void FogEffect::DrawFog()
{
    // 深度が無いときは描けない(PostEffectPipeline が有効な間は深度を渡している)
    if (sceneTextures == nullptr || sceneTextures->depth == nullptr)
    {
        return;
    }

    // カメラは動くので、ワールド座標を求めるための行列とカメラ位置を毎フレーム反映する
    auto camera = Camera3D::GetInstance();

    auto& p = param.Edit();

    // (ビュー * 射影) の逆行列。HLSL は行列を列優先で読むので、転置して渡し、シェーダー側は mul(ベクトル, 行列) で使う
    const DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(camera->GetViewMatrix(), camera->GetProjectionMatrix());

    const DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);

    DirectX::XMStoreFloat4x4(&p.invViewProj, DirectX::XMMatrixTranspose(invViewProj));

    p.cameraPos = camera->GetPosition();

    DrawTwoInputs(inputTexture, sceneTextures->depth, param.GetBuffer());
}

void FogEffect::DrawTwoInputs(DescriptorHandle* _t0, DescriptorHandle* _t1, ConstBuffer* _constBuffer)
{
    if (pipelineState == nullptr || _t0 == nullptr || _t1 == nullptr || _constBuffer == nullptr)
    {
        return;
    }

    auto commandList = Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(0, _t0->handleGPU);
    commandList->SetGraphicsRootDescriptorTable(1, _t1->handleGPU);
    commandList->SetGraphicsRootConstantBufferView(2, _constBuffer->GetAddress());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);
}

void FogEffect::SetColor(const Vector4& _color)
{
    auto& p = param.Edit();

    p.color = _color;
}

Vector4 FogEffect::GetColor(const Vector4& _color) const
{
    const auto& p = param.Get();

    return p.color;
}

void FogEffect::SetDensity(float _density)
{
    param.Edit().density = (_density < 0.0f) ? 0.0f : _density;
}

void FogEffect::SetSkyAmount(float _amount)
{
    param.Edit().skyAmount = std::clamp(_amount, 0.0f, 1.0f);
}

void FogEffect::SetHeightFalloff(float _falloff)
{
    param.Edit().heightFalloff = (_falloff < 0.0f) ? 0.0f : _falloff;
}

void FogEffect::SetBaseHeight(float _height)
{
    param.Edit().baseHeight = _height;
}

void FogEffect::SetStartDistance(float _distance)
{
    param.Edit().startDistance = (_distance < 0.0f) ? 0.0f : _distance;
}

void FogEffect::SetMaxOpacity(float _opacity)
{
    param.Edit().maxOpacity = std::clamp(_opacity, 0.0f, 1.0f);
}
