#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"

class RenderTarget;

// 光らせたい部分(シーン描画で SV_TARGET1 に書いたもの)だけをぼかして、元の画像に足すブルーム
//
//  パス1 : Bloomターゲットを横ぼかし(同時に threshold 以下の暗い部分を捨てる) -> 中間RT A
//  パス2 : 中間RT A を縦ぼかし -> 中間RT B
//  パス3 : 入力画像(前段の結果) + 中間RT B * intensity -> 最終出力
//
//  ・「Bloomターゲット」は PostEffectPipeline が用意するシーンのMRT(2枚目)。
//    FBXModel などが描画時に書き込む(書かないオブジェクトは光らない)
//  ・パスごとにシェーダーは違うが、ルートシグネチャは共通(t0, t1, b0)
struct BloomBlurParam
{
    float weights[8] = {};
    float direction[2] = {};

    //サンプル間隔(テクセル倍率) 大きいほど広がる
    float offsetScale = 1.0f;

    //この明るさ(輝度)以下は捨てる。0以上で有効、マイナスなら素通し(縦ぼかしで使う)
    float threshold = -1.0f;
};

struct BloomCompositeParam
{
    float intensity = 1.0f;
    float padding[3] = {};
};

class BloomEffect : public PostEffect
{
public:

    BloomEffect();
    ~BloomEffect();

    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    //この明るさ(0〜1)を超えた分だけが光る
    void SetThreshold(float _threshold);
    float GetThreshold() const { return threshold; }

    //光の強さ(合成するときの倍率)
    void SetIntensity(float _intensity);
    float GetIntensity() const { return intensity; }

    //ぼかしの重みの広がり
    void SetSigma(float _sigma);
    float GetSigma() const { return sigma; }

    //ぼかしのサンプル間隔(テクセル倍率)  大きいほど広く光る
    void SetSpread(float _spread);
    float GetSpread() const { return spread; }

private:

    void DrawBlurHorizontal();
    void DrawBlurVertical();
    void DrawComposite();

    //t0 / t1 / b0 を設定してフルスクリーン描画(t1 が無いときは t0 を代わりに渡す)
    void DrawTwoInputs(PipelineState* _pipelineState, DescriptorHandle* _t0, DescriptorHandle* _t1, ConstBuffer* _constBuffer);

    //threshold / sigma / spread / intensity を定数バッファへ反映する
    void ApplyParam();

    float threshold = 0.6f;
    float intensity = 1.0f;
    float sigma = 5.0f;
    float spread = 3.0f;

    //ぼかし用 : pipelineState(基底クラス)は合成用に使う
    std::unique_ptr<PipelineState> blurPipelineState;

    //ぼかしの中間描画先(横 -> 縦)
    std::unique_ptr<RenderTarget> blurTargetA;
    std::unique_ptr<RenderTarget> blurTargetB;

    PostEffectParam<BloomBlurParam> horizontalParam;
    PostEffectParam<BloomBlurParam> verticalParam;
    PostEffectParam<BloomCompositeParam> compositeParam;
};
