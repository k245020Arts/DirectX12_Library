#pragma once
#include "../PostEffect.h"

class RenderTarget;

struct GaussianBlurParam
{
    float bkWeights[8] = {};
    float direction[2] = {};
};

class GaussianBlur : public PostEffect
{
public:

    GaussianBlur();
    ~GaussianBlur();
    bool Init() override;

    void Draw() override;

    void DrawHorizontal();
    void DrawVertical();

    RenderTarget* GetIntermediateRenderTarget();

private:

    std::vector<float> GetGaussianWeights(size_t count, float s);

    GaussianBlurParam param;

    // 横方向の中間描画先
    std::unique_ptr<RenderTarget> intermediateRenderTarget;

    // 横方向用定数バッファ
    std::vector<std::shared_ptr<ConstBuffer>> horizontalConstBuffers;

    // 縦方向用定数バッファ
    std::vector<std::shared_ptr<ConstBuffer>> verticalConstBuffers;
};

