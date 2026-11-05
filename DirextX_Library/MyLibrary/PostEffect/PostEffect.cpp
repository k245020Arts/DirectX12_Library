#include "PostEffect.h"
#include "../Engine/Engine.h"
#include "../RootSignature/RootSignature.h"
#include "../PipelineState/PipelineState.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../ConstBuffer/ConstBuffer.h"

PostEffect::PostEffect()
{
}

PostEffect::~PostEffect()
{
}

bool PostEffect::Init()
{
    postEffectConstBuff.resize(FRAME_BUFFER_COUNT);
    for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        postEffectConstBuff[i] = std::make_shared<ConstBuffer>(sizeof(PostEffectParam));
        if (!postEffectConstBuff[i]->IsValid())
        {
            OutputDebugStringW(L"ポストエフェクト用定数バッファー生成に失敗\n");
        }
    }

    auto weights = GetGaussianWeights(8, 5.0f);

    for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        auto ptr = postEffectConstBuff[i]->GetPtr<PostEffectParam>();
        std::copy(weights.begin(), weights.end(), ptr->bkWeights);
    }

    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/PostEffectBasePixelShader.hlsl", "PostEffectBasePSShader");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
    }

    return true;
}

void PostEffect::SetInputTexture(
    DescriptorHandle* handle
)
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

    auto heap =DescriptorHeap::GetInstance()->GetHeap();

    auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps );

    commandList->SetGraphicsRootSignature(rootSignature->Get());
    commandList->SetGraphicsRootConstantBufferView(0, postEffectConstBuff[currentIndex]->GetAddress());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(1,inputTexture->handleGPU);

    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    commandList->DrawInstanced(3,1,0,0);
}

std::vector<float> PostEffect::GetGaussianWeights(size_t count, float s)
{
    std::vector<float> weights(count);
    float x = 0.0f;
    float total = 0.0f;
    for (auto& wgt : weights) {
        wgt = expf(-(x * x) / (2 * s * s));
        total += wgt;
        x += 1.0f;
    }
    //真ん中を中心に左右に広がるように作る
    //左右なので2倍。しかしその場合は中心の0のピクセルが
    //重複するから、e^0=1ということで最後に1を引いて辻褄が合うようにしてる。
    total = total * 2.0f - 1.0f;
    //足して１になるようにする
    for (auto& wgt : weights) {
        wgt /= total;
    }
    return weights;
}
