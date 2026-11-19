#pragma once
#include <DirectXMath.h>
#include "../PostEffect.h"
#include "../PostEffectParam.h"

// C++ と HLSL(Fog/FogPixelShader.hlsl の cbuffer FogParam)で同じ並びにすること(128byte)
struct FogParam
{
    Vector4 color = { 0.7f, 0.8f, 0.9f, 1.0f };

    Vector3 cameraPos;  // カメラのワールド座標(毎フレーム Camera3D から反映する)
    float density = 0.01f;    // 基準の高さ(baseHeight)での濃さ

    float heightFalloff = 0.05f; // 高くなるほど薄くなる速さ。大きいほど地面付近にだけフォグが溜まる。0 なら高さに関係なく一様
    float baseHeight = 0.0f;     // フォグの濃さの基準になる高さ(Y)
    float maxOpacity = 1.0f;     // フォグの最大の濃さ 0〜1(1 未満なら、どんなに遠くても景色が少し見える)
    float startDistance = 0.0f;  // カメラからこの距離まではフォグをかけない

    float skyAmount = 1.0f; // 何も描かれていない場所(空)にフォグをかける割合 0〜1
    Vector3 fogPad = {};

    // 画面のピクセルとその深度から、ワールド座標を求めるための行列(毎フレーム Camera3D から計算して入れる)
    DirectX::XMFLOAT4X4 invViewProj = {};
};

// 指数関数的な高さフォグ(Exponential Height Fog)
//
//  ・密度が高さに応じて指数的に薄くなる(地面付近が濃く、上空は薄い)霧を、カメラから各ピクセルまでの視線に沿って積分する
//    現実の霧・靄に近い見え方になる。ゲームエンジン(UE など)の標準的なフォグと同じ考え方
//  ・シェーダーに分岐(if)は無い(式だけで計算する)
//  ・ワールド座標は、Engine の深度バッファのコピー(SceneTextures::depth)から求める
//  ・PostEffectPipeline が「このエフェクトは深度が必要」と見て、有効な間だけ自動で深度をコピーする
//  ・ライト視点のシャドウマップ深度では、カメラからの距離は分からないので使えない
class FogEffect : public PostEffect
{
public:

    FogEffect();
    ~FogEffect();

    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    // 深度テクスチャが必要なエフェクト
    bool NeedsSceneDepth() const override { return true; }

    void SetColor(const Vector4& _color);
    Vector4 GetColor(const Vector4& _color) const;

    void SetDensity(float _density);
    float GetDensity() const { return param.Get().density; }

    void SetHeightFalloff(float _falloff);
    float GetHeightFalloff() const { return param.Get().heightFalloff; }

    void SetBaseHeight(float _height);
    float GetBaseHeight() const { return param.Get().baseHeight; }

    void SetStartDistance(float _distance);
    float GetStartDistance() const { return param.Get().startDistance; }

    void SetMaxOpacity(float _opacity);
    float GetMaxOpacity() const { return param.Get().maxOpacity; }

    // 空(何も描かれていない場所)にかける割合。0 で空にはかけない
    void SetSkyAmount(float _amount);
    float GetSkyAmount() const { return param.Get().skyAmount; }

private:

    void DrawFog();

    // t0(色) と t1(深度) を設定してフルスクリーン描画
    void DrawTwoInputs(DescriptorHandle* _t0, DescriptorHandle* _t1, ConstBuffer* _constBuffer);

    PostEffectParam<FogParam> param;
};
