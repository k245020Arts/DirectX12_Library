#include "Vignette.h"
#include <algorithm>
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../Camera3D/Camera3D.h"

Vignette::Vignette()
{
}

Vignette::~Vignette()
{
}

bool Vignette::Init()
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
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/Vignette/VignettePixelShader.hlsl", "VignettePS");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"Vignette : パイプラインステートの生成に失敗\n");
        return false;
    }

    return true;
}

void Vignette::Draw()
{
}

std::vector<PostEffectPassInfo> Vignette::CreatePasses()
{
    return { { [this]() { DrawVignette(); }, nullptr } };
}

void Vignette::DrawVignette()
{
    auto& p = param.Edit();

    auto camera = Camera3D::GetInstance();

    p.aspectRatio = camera->GetAspect();

    p.time += DeltaTime::GetInstance()->GetDeltaTimeMulTimeScale();

    auto commandList = Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootConstantBufferView(0, param.GetBuffer()->GetAddress());
    commandList->SetGraphicsRootDescriptorTable(1, inputTexture->handleGPU);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);
}
