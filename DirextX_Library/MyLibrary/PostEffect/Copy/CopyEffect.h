#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"

struct CopyParam
{
    float padding[4] = {};
};

// 入力テクスチャをそのまま出力先へ描くだけのエフェクト
// PostEffectPipeline が内部で使う(有効なエフェクトが1つも無いとき、シーンの結果をBackBufferへ出すため)
class CopyEffect : public PostEffect
{
public:

    CopyEffect();
    ~CopyEffect();

    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

private:

    PostEffectParam<CopyParam> param;
};
