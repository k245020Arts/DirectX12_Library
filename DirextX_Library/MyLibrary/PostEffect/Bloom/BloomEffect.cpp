#include "BloomEffect.h"
#include <algorithm>
#include <cmath>
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../RenderTarget/RenderTarget.h"

namespace
{
    // 中心から片側 count 個分のガウス重み(合計が1になる)
    std::vector<float> GetGaussianWeights(size_t _count, float _sigma)
    {
        std::vector<float> weights(_count);

        float x = 0.0f;
        float total = 0.0f;

        for (auto& w : weights)
        {
            w = expf(-(x * x) / (2.0f * _sigma * _sigma));
            total += w;
            x += 1.0f;
        }

        // 左右に広がるので2倍。中心が重複するので1を引く
        total = total * 2.0f - 1.0f;

        for (auto& w : weights)
        {
            w /= total;
        }

        return weights;
    }

    // ぼかし/合成で共通のパイプラインステートを作る
    std::unique_ptr<PipelineState> CreatePostEffectPipeline(RootSignature* _rootSignature, const wchar_t* _psFile, const char* _psEntry)
    {
        auto pipeline = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

        pipeline->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
        pipeline->SetRootSignature(_rootSignature->Get());
        pipeline->SetDepthEnable(false);
        pipeline->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
        pipeline->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
        pipeline->SetPS(_psFile, _psEntry);
        pipeline->Create();

        if (!pipeline->IsSuccess())
        {
            OutputDebugStringW(L"BloomEffect : パイプラインステートの生成に失敗\n");
        }

        return pipeline;
    }
}

BloomEffect::BloomEffect()
{
}

BloomEffect::~BloomEffect()
{
}

bool BloomEffect::Init()
{
    auto size = Engine::GetInstance()->GetWindowSize();

    // ぼかしの中間RT(黒でクリアする。明るい部分だけを扱うので)
    const Vector4 black(0.25f, 0.25f, 0.25f, 0.0f);

    blurTargetA = std::make_unique<RenderTarget>();
    blurTargetB = std::make_unique<RenderTarget>();

    if (!blurTargetA->Create(size.width, size.height, DXGI_FORMAT_R8G8B8A8_UNORM, black) ||
        !blurTargetB->Create(size.width, size.height, DXGI_FORMAT_R8G8B8A8_UNORM, black))
    {
        return false;
    }

    // 定数バッファ(フレーム数分)
    if (!horizontalParam.Create() || !verticalParam.Create() || !compositeParam.Create())
    {
        return false;
    }

    // 方向は固定。threshold は横ぼかしだけで使う(縦はマイナス = 素通し)
    horizontalParam.Edit().direction[0] = 1.0f;
    horizontalParam.Edit().direction[1] = 0.0f;

    verticalParam.Edit().direction[0] = 0.0f;
    verticalParam.Edit().direction[1] = 1.0f;
    verticalParam.Edit().threshold = -1.0f;

    ApplyParam();

    // ルートシグネチャ : t0, t1, b0 (サンプラーは LINEAR + CLAMP)
    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetPostEffectTwoInputRootSignature();

    blurPipelineState = CreatePostEffectPipeline(rootSignature.get(), L"Shader/PostEffectBaseShader/Bloom/BloomBlurPixelShader.hlsl", "BloomBlurPS");

    pipelineState = CreatePostEffectPipeline(rootSignature.get(), L"Shader/PostEffectBaseShader/Bloom/BloomCompositePixelShader.hlsl", "BloomCompositePS");

    return true;
}

void BloomEffect::Draw()
{
    // パスごとの関数(DrawBlurHorizontal など)で描くので、ここでは何もしない
}

std::vector<PostEffectPassInfo> BloomEffect::CreatePasses()
{
    std::vector<PostEffectPassInfo> passes;

    // パス1 : 横ぼかし(+ 暗い部分を捨てる) -> 中間RT A
    passes.push_back({ [this]() { DrawBlurHorizontal(); }, blurTargetA.get() });

    // パス2 : 縦ぼかし -> 中間RT B
    passes.push_back({ [this]() { DrawBlurVertical(); }, blurTargetB.get() });

    // パス3 : 入力画像 + ぼかした光 -> 最終出力(出力先はPipelineが決める)
    passes.push_back({ [this]() { DrawComposite(); }, nullptr });

    return passes;
}

void BloomEffect::DrawBlurHorizontal()
{
    // シーンの Bloom ターゲットが無ければ何もしない
    if (sceneTextures == nullptr || sceneTextures->bloom == nullptr)
    {
        return;
    }

    DrawTwoInputs(blurPipelineState.get(), sceneTextures->bloom, nullptr, horizontalParam.GetBuffer());
}

void BloomEffect::DrawBlurVertical()
{
    DrawTwoInputs(blurPipelineState.get(), blurTargetA->GetSRV(), nullptr, verticalParam.GetBuffer());
}

void BloomEffect::DrawComposite()
{
    // t0 : 入力画像(前段の結果)  t1 : ぼかした光
    DrawTwoInputs(pipelineState.get(), inputTexture, blurTargetB->GetSRV(), compositeParam.GetBuffer());
}

void BloomEffect::DrawTwoInputs(PipelineState* _pipelineState, DescriptorHandle* _t0, DescriptorHandle* _t1, ConstBuffer* _constBuffer)
{
    if (_pipelineState == nullptr || _t0 == nullptr || _constBuffer == nullptr)
    {
        return;
    }

    if (_t1 == nullptr)
    {
        _t1 = _t0;
    }

    auto commandList = Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());

    commandList->SetPipelineState(_pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(0, _t0->handleGPU);
    commandList->SetGraphicsRootDescriptorTable(1, _t1->handleGPU);
    commandList->SetGraphicsRootConstantBufferView(2, _constBuffer->GetAddress());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);
}

void BloomEffect::SetThreshold(float _threshold)
{
    threshold = std::clamp(_threshold, 0.0f, 0.99f);

    ApplyParam();
}

void BloomEffect::SetIntensity(float _intensity)
{
    intensity = (_intensity < 0.0f) ? 0.0f : _intensity;

    ApplyParam();
}

void BloomEffect::SetSigma(float _sigma)
{
    sigma = (_sigma < 0.01f) ? 0.01f : _sigma;

    ApplyParam();
}

void BloomEffect::SetSpread(float _spread)
{
    spread = (_spread < 0.0f) ? 0.0f : _spread;

    ApplyParam();
}

void BloomEffect::ApplyParam()
{
    const auto weights = GetGaussianWeights(8, sigma);

    auto apply = [&](PostEffectParam<BloomBlurParam>& _param)
    {
        auto& p = _param.Edit();

        std::copy(weights.begin(), weights.end(), p.weights);

        p.offsetScale = spread;
    };

    apply(horizontalParam);
    apply(verticalParam);

    // threshold は横ぼかしだけ(縦は -1 のまま)
    horizontalParam.Edit().threshold = threshold;

    compositeParam.Edit().intensity = intensity;
}
