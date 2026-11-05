#include "PostEffect.h"
#include "../Engine/Engine.h"
#include "../RootSignature/RootSignature.h"
#include "../PipelineState/PipelineState.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../ConstBuffer/ConstBuffer.h"

PostEffect::PostEffect()
{
    rootSignature = nullptr;

    pipelineState = nullptr;

}

PostEffect::~PostEffect()
{
}

void PostEffect::SetInputTexture(DescriptorHandle* handle)
{
    inputTexture = handle;
}

void PostEffect::Draw()
{
    if (inputTexture == nullptr)
    {
        return;
    }

    auto commandList =Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps );

    commandList->SetGraphicsRootSignature(rootSignature->Get());
    //commandList->SetGraphicsRootConstantBufferView(0, postEffectConstBuff[currentIndex]->GetAddress());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(1,inputTexture->handleGPU);

    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    commandList->DrawInstanced(3,1,0,0);
}

void PostEffect::DrawFullscreen(DescriptorHandle* inputTexture, ConstBuffer* constBuffer)
{
    if (inputTexture == nullptr || constBuffer == nullptr)
    {
        return;
    }

    auto engine = Engine::GetInstance();

    auto commandList = engine->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    // DescriptorHeap設定
    commandList->SetDescriptorHeaps(1, heaps);

    // RootSignature
    commandList->SetGraphicsRootSignature(rootSignature->Get());

    // 定数バッファ
    commandList->SetGraphicsRootConstantBufferView(0,constBuffer->GetAddress());

    // PipelineState
    commandList->SetPipelineState( pipelineState->GetPipelineState());

    // 入力テクスチャ
    commandList->SetGraphicsRootDescriptorTable(1,inputTexture->handleGPU);

    // トポロジー
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // フルスクリーン描画
    commandList->DrawInstanced(3, 1, 0, 0);
}