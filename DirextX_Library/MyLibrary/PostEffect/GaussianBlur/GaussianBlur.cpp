#include "GaussianBlur.h"
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../RenderTarget/RenderTarget.h"

GaussianBlur::GaussianBlur()
{

}

GaussianBlur::~GaussianBlur()
{

}

bool GaussianBlur::Init()
{
    auto engine = Engine::GetInstance();

    auto size = engine->GetWindowSize();

    // 中間レンダーターゲット生成
    intermediateRenderTarget = std::make_unique<RenderTarget>();

    if (!intermediateRenderTarget->Create(size.width,size.height,DXGI_FORMAT_R8G8B8A8_UNORM))
    {
        return false;
    }

    // 定数バッファ生成
    horizontalConstBuffers.resize(FRAME_BUFFER_COUNT);
    verticalConstBuffers.resize(FRAME_BUFFER_COUNT);

    auto weights = GetGaussianWeights(8, 5.0f);

    for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        horizontalConstBuffers[i] =std::make_shared<ConstBuffer>(sizeof(GaussianBlurParam));

        verticalConstBuffers[i] =std::make_shared<ConstBuffer>(sizeof(GaussianBlurParam));

        if (!horizontalConstBuffers[i]->IsValid() || !verticalConstBuffers[i]->IsValid())
        {
            return false;
        }

        // 横方向
        auto horizontal = horizontalConstBuffers[i]->GetPtr<GaussianBlurParam>();

        std::copy(weights.begin(),weights.end(),horizontal->bkWeights);

        horizontal->direction[0] = 1.0f;
        horizontal->direction[1] = 0.0f;

        // 縦方向
        auto vertical =verticalConstBuffers[i]->GetPtr<GaussianBlurParam>();

        std::copy(weights.begin(),weights.end(),vertical->bkWeights);

        vertical->direction[0] = 0.0f;
        vertical->direction[1] = 1.0f;
    }

    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/GaussianBlurPixelShader.hlsl", "GaussianBlurPSShader");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
    }

    return true;
}

void GaussianBlur::Draw()
{
    
}

void GaussianBlur::DrawHorizontal()
{
    auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
    // 元のテクスチャを使って水平方向のブラー
    DrawFullscreen(inputTexture, horizontalConstBuffers[currentIndex].get());
}

void GaussianBlur::DrawVertical()
{
    auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
    // 中間RenderTargetを使って垂直方向のブラー
    DrawFullscreen(intermediateRenderTarget->GetSRV(),verticalConstBuffers[currentIndex].get());
}

RenderTarget* GaussianBlur::GetIntermediateRenderTarget()
{
    return intermediateRenderTarget.get();
}

std::vector<float> GaussianBlur::GetGaussianWeights(size_t count, float s)
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
