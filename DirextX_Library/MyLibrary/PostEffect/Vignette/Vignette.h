#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"

struct VignetteParam
{
    Vector3 vignetteColor = Vector3(0,0,0);   // ビネットの色 (黒なら float3(0,0,0)、ダメージなら float3(0.6,0,0))
    float intensity = 1.0f;        // 暗さ・色の濃さの強さ (標準: 1.0f ~ 1.5f)

    Vector2 centerPos = Vector2(0.5f,0.5f);       // ビネットの中心座標 (標準: float2(0.5f, 0.5f))
    float smoothness = 0.4f;       // グラデーションの滑らかさ (標準: 0.2f ~ 0.6f)
    float roundness = 1.0f;        // 真円度 (0.0f = 画面比率に沿った楕円, 1.0f = 正円)

    float aspectRatio = 0.0f;      // 画面のアスペクト比 (Width / Height 例: 16.0f / 9.0f)
    float time = 0.0f;
    Vector2 padding;         // 16バイト境界合わせ用パディング
};

class Vignette : public PostEffect
{
public:
    Vignette();
    ~Vignette();

    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    void DrawVignette();

    // --- セッター (Setter) ---
    void SetVignetteColor(const Vector3& color) { param.Edit().vignetteColor = color; }
    void SetVignetteColor(float r, float g, float b) { param.Edit().vignetteColor = Vector3(r, g, b); }
    void SetIntensity(float intensity) { param.Edit().intensity = intensity; }
    void SetCenterPos(const Vector2& pos) { param.Edit().centerPos = pos; }
    void SetCenterPos(float x, float y) { param.Edit().centerPos = Vector2(x, y); }
    void SetSmoothness(float smoothness) { param.Edit().smoothness = smoothness; }
    void SetRoundness(float roundness) { param.Edit().roundness = roundness; }

    // --- ゲッター (Getter) ---
    const Vector3& GetVignetteColor() const { return param.Get().vignetteColor; }
    float GetIntensity() const { return param.Get().intensity; }
    const Vector2& GetCenterPos() const { return param.Get().centerPos; }
    float GetSmoothness() const { return param.Get().smoothness; }
    float GetRoundness() const { return param.Get().roundness; }

private:

    PostEffectParam<VignetteParam> param;
};

