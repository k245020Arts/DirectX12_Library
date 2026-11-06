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

    // 定数バッファ生成(フレーム数分)
    if (!horizontalParam.Create() || !verticalParam.Create())
    {
        return false;
    }

    // 方向は固定
    horizontalParam.Edit().direction[0] = 1.0f;
    horizontalParam.Edit().direction[1] = 0.0f;

    verticalParam.Edit().direction[0] = 0.0f;
    verticalParam.Edit().direction[1] = 1.0f;

    // 重みとサンプル間隔
    ApplyParam();

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

std::vector<PostEffectPassInfo> GaussianBlur::CreatePasses()
{
    std::vector<PostEffectPassInfo> passes;

    // 水平方向のブラー : 中間RenderTargetへ
    passes.push_back({ [this]() { DrawHorizontal(); }, intermediateRenderTarget.get() });

    // 垂直方向のブラー : 最終出力(出力先はPipelineが決める)
    passes.push_back({ [this]() { DrawVertical(); }, nullptr });

    return passes;
}

void GaussianBlur::DrawHorizontal()
{
    // 元のテクスチャを使って水平方向のブラー
    DrawFullscreen(inputTexture, horizontalParam.GetBuffer());
}

void GaussianBlur::DrawVertical()
{
    // 中間RenderTargetを使って垂直方向のブラー
    DrawFullscreen(intermediateRenderTarget->GetSRV(), verticalParam.GetBuffer());
}

void GaussianBlur::SetSigma(float _sigma)
{
    sigma = (_sigma < 0.01f) ? 0.01f : _sigma;

    ApplyParam();
}

void GaussianBlur::SetSpread(float _spread)
{
    spread = (_spread < 0.0f) ? 0.0f : _spread;

    ApplyParam();
}

void GaussianBlur::ApplyParam()
{
    auto weights = GetGaussianWeights(8, sigma);

    auto apply = [&](PostEffectParam<GaussianBlurParam>& _param)
    {
        auto& p = _param.Edit();

        std::copy(weights.begin(), weights.end(), p.bkWeights);

        p.offsetScale = spread;
    };

    apply(horizontalParam);
    apply(verticalParam);
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
