#include "CopyEffect.h"
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"

CopyEffect::CopyEffect()
{
}

CopyEffect::~CopyEffect()
{
}

bool CopyEffect::Init()
{
    if (!param.Create())
    {
        return false;
    }

    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/Bloom/CopyPixelShader.hlsl", "CopyPS");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"CopyEffect : パイプラインステートの生成に失敗\n");
        return false;
    }

    return true;
}

void CopyEffect::Draw()
{
    DrawFullscreen(inputTexture, param.GetBuffer());
}

std::vector<PostEffectPassInfo> CopyEffect::CreatePasses()
{
    // 1パスで完結する最終出力パスのみ
    return { { [this]() { Draw(); }, nullptr } };
}
