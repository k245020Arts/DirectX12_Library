#include "MotionBlur.h"
#include <algorithm>
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../Camera3D/Camera3D.h"

MotionBlur::MotionBlur()
{

}

MotionBlur::~MotionBlur()
{

}

bool MotionBlur::Init()
{
    if (!param.Create())
    {
        return false;
    }

    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectTwoInputRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/MotionBlur/MotionBlurPixelShader.hlsl", "MotionBlurPS");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"MosionBlur : パイプラインステートの生成に失敗\n");
        return false;
    }

    return true;
}

void MotionBlur::Draw()
{
    DrawMotionBlur();
}

std::vector<PostEffectPassInfo> MotionBlur::CreatePasses()
{
    // 1パスで完結する最終出力パスのみ
    return { { [this]() { DrawMotionBlur(); }, nullptr } };
}

void MotionBlur::DrawMotionBlur()
{
    // 深度が無いときは描けない(PostEffectPipeline が有効な間は深度を渡している)
    if (sceneTextures == nullptr || sceneTextures->depth == nullptr)
    {
        return;
    }

    // カメラは動くので、ワールド座標を求めるための行列とカメラ位置を毎フレーム反映する
    auto camera = Camera3D::GetInstance();

    auto& p = param.Edit();

    const DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    
    const DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);

    DirectX::XMStoreFloat4x4(&p.gInvCurrViewProj, DirectX::XMMatrixTranspose(invViewProj));

    const DirectX::XMMATRIX prevViewProj = DirectX::XMMatrixMultiply(camera->GetPrevViewMatrix(), camera->GettPrevProjectionMatrix());

    DirectX::XMStoreFloat4x4(&p.gPrevViewProj, DirectX::XMMatrixTranspose(prevViewProj));

    auto commandList = Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(0, inputTexture->handleGPU);
    commandList->SetGraphicsRootDescriptorTable(1, sceneTextures->depth->handleGPU);
    commandList->SetGraphicsRootConstantBufferView(2, param.GetBuffer()->GetAddress());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);
}
