#include "ShadowMapDebug.h"
#include "ShadowMap.h"
#include "../Engine/Engine.h"
#include "../RootSignature/RootSignature.h"
#include "../PipelineState/PipelineState.h"
#include "../DescriptorHeap/DescriptorHeap.h"

ShadowMapDebug::ShadowMapDebug()
{

}

ShadowMapDebug::~ShadowMapDebug()
{
}


bool ShadowMapDebug::Init()
{
    rootSignature = std::make_unique<RootSignature>();
    rootSignature->SetShadowDebugRootSignature();
    if (!rootSignature->IsSuccess())
    {
        OutputDebugStringW(L"ShadowMapDebug: RootSignatureの生成に失敗\n");
        return false;
    }

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{});
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl","PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/Shadow/ShadowMapDebugPixelShader.hlsl","ShadowMapDebugPS");
    pipelineState->PostEffectPipelineCreate();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"ShadowMapDebug: PipelineStateの生成に失敗\n");
        return false;
    }

    return true;
}

bool ShadowMapDebug::IsValid() const
{
    return rootSignature != nullptr && rootSignature->IsSuccess() &&
        pipelineState != nullptr && pipelineState->IsSuccess();
}

void ShadowMapDebug::Draw(const ShadowMap& shadowMap)
{
    if (!IsValid() || !shadowMap.IsValid() || shadowMap.GetSRV() == nullptr)
    {
        return;
    }

    auto* commandList = Engine::GetInstance()->CommandList();
    auto heap = DescriptorHeap::GetInstance()->GetHeap();
    if (heap.Get() == nullptr)
    {
        return;
    }

    ID3D12DescriptorHeap* heaps[] = { heap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());
    commandList->SetPipelineState(pipelineState->GetPipelineState());
    commandList->SetGraphicsRootDescriptorTable(0, shadowMap.GetSRV()->handleGPU);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}
