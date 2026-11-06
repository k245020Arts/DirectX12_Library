#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"

class RenderTarget;

struct GaussianBlurParam
{
    float bkWeights[8] = {};
    float direction[2] = {};

    //サンプル間隔(テクセル倍率) 大きいほど広範囲をぼかす
    float offsetScale = 1.0f;

    float padding = 0.0f;
};

class GaussianBlur : public PostEffect
{
public:

    GaussianBlur();
    ~GaussianBlur();
    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    void DrawHorizontal();
    void DrawVertical();

    RenderTarget* GetIntermediateRenderTarget();

    //ブラーの強さ(重みの広がり)  小さいほど中心寄り、大きいほど均一にぼける
    void SetSigma(float _sigma);
    float GetSigma() const { return sigma; }

    //ブラーのサンプル間隔(テクセル倍率)  大きいほど広範囲をぼかす(大きすぎると粗くなる)
    void SetSpread(float _spread);
    float GetSpread() const { return spread; }

private:

    std::vector<float> GetGaussianWeights(size_t count, float s);

    //sigma / spread を横方向・縦方向の定数バッファへ反映する
    void ApplyParam();

    float sigma = 5.0f;
    float spread = 1.0f;

    // 横方向の中間描画先
    std::unique_ptr<RenderTarget> intermediateRenderTarget;

    // 横方向用パラメータ(定数バッファ)
    PostEffectParam<GaussianBlurParam> horizontalParam;

    // 縦方向用パラメータ(定数バッファ)
    PostEffectParam<GaussianBlurParam> verticalParam;
};

