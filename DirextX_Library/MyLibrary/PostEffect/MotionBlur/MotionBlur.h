#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"

class RenderTarget;

struct MotionBlurParam
{
    DirectX::XMFLOAT4X4 gInvCurrViewProj; // 現フレームのViewProj逆行列
    DirectX::XMFLOAT4X4 gPrevViewProj;    // 前フレームのViewProj行列
    float gBlurStrength = 1.0f; // ブラーの強さ（例: 0.5 ~ 1.0）
    int   gSampleCount = 5;  // サンプル数（例: 8 ~ 16）
    Vector2 motionPadding;
};


class MotionBlur : public PostEffect
{
public:
    MotionBlur();
    ~MotionBlur();

    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    void SetStrength(float _strngth) { param.Edit().gBlurStrength = _strngth; }
    float GetStrength() { return param.Get().gBlurStrength; };

    void SetSampleCount(int _sampleCount) { param.Edit().gSampleCount = _sampleCount; }
    int GetSampleCount() { return param.Get().gSampleCount; };

    bool NeedsSceneDepth() const override { return true; }
private:

    PostEffectParam<MotionBlurParam> param;

    void DrawMotionBlur();
};

