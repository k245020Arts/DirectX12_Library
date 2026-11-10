#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <type_traits>
#include <utility>
#include <wtypes.h>
#include "PostEffect.h"
#include "../RenderTarget/RenderTarget.h"
#include "../RenderPass/RenderPass.h"

//ポストエフェクトのチェーンを管理するクラス
//
//使い方
//  pipeline.Initialize(width, height);
//  pipeline.SetSceneFunction([]() { 描画処理 });
//  auto* blur = pipeline.AddEffect<GaussianBlur>();   // 追加した順番にかかる
//  pipeline.AddEffect<DistortionPostEffect>();
//  pipeline.Build();                                  // RenderTarget / RenderPass を自動構築して登録
//
//  blur->SetEnabled(false);   // 個別にON/OFF(つなぎ直しは自動)
//  blur->SetSigma(3.0f);      // エフェクト固有のパラメータ
//
//自動でやること
//  ・シーンの描画先RenderTargetの用意(有効なエフェクトが無ければBackBufferへ直接描画)
//  ・エフェクト間の中間RenderTargetの用意(2枚を交互に使い回す)
//  ・各エフェクトへの入力テクスチャの設定
//  ・有効なエフェクトのうち最後のものの出力先をBackBufferにする
//  ・RenderPassの生成と RenderPassManager への登録
//  ・エフェクトの有効/無効が切り替わったときの入出力のつなぎ直し
//
//新しいエフェクトを増やすとき
//  PostEffectを継承して Init / Draw / CreatePasses を実装するだけで、このクラスは変更不要
class PostEffectPipeline
{
public:

    PostEffectPipeline() {};

    ~PostEffectPipeline() {};

    //初期化
    bool Init(UINT _width, UINT _height);

    //シーン描画処理の設定
    void SetSceneFunction(std::function<void()> _function);

    //エフェクト追加(Build前のみ)
    //追加した順番がそのまま適用順になる
    //生成とInitまで行い、設定用に生ポインタを返す(失敗時は nullptr)
    //  例) auto* distortion = pipeline.AddEffect<DistortionPostEffect>();
    //      distortion->Load("data/testDist.png");
    //返したポインタの所有権はPipelineが持つので delete しないこと
    template<class T, class... Args>
    T* AddEffect(Args&&... _args)
    {
        static_assert(std::is_base_of<PostEffect, T>::value, "T は PostEffect を継承してください");

        if (!isInitialized || isBuilt)
        {
            return nullptr;
        }

        auto effect = std::make_unique<T>(std::forward<Args>(_args)...);

        if (!effect->Init())
        {
            return nullptr;
        }

        T* raw = effect.get();

        //有効/無効が切り替わったら、つなぎ直す
        raw->SetStateChangedCallback([this]()
        {
            if (isBuilt)
            {
                ApplyRouting();
            }
        });

        EffectEntry entry;

        entry.effect = std::move(effect);

        entries.push_back(std::move(entry));

        return raw;
    }

    //RenderTarget と RenderPass を構築して RenderPassManager に登録する
    bool Build();

    //取得
    RenderTarget* GetSceneRenderTarget() const
    {
        return sceneRenderTarget.get();
    }

    size_t GetEffectCount() const
    {
        return entries.size();
    }

private:

    //エフェクト1つ分の情報
    struct EffectEntry
    {
        std::unique_ptr<PostEffect> effect;

        //エフェクトが宣言したパス情報(中間RTなど)
        std::vector<PostEffectPassInfo> infos;

        //infos と同じ並びのRenderPass
        std::vector<std::shared_ptr<RenderPass>> passes;
    };

    //エフェクト間の中間RenderTarget(0 or 1)を必要になったときに生成して返す
    RenderTarget* GetWorkRenderTarget(size_t _index);

    //有効なエフェクトだけで 入力 -> 出力 のつなぎと、各RenderPassの有効/描画先を決め直す
    void ApplyRouting();

    UINT width = 0;
    UINT height = 0;

    bool isInitialized = false;
    bool isBuilt = false;

    //シーン描画処理
    std::function<void()> sceneFunction;

    //※ メンバーは後ろから破棄される
    //  RenderTarget -> entries の順に宣言して、参照される側を後に破棄する

    //シーン描画先
    std::shared_ptr<RenderTarget> sceneRenderTarget;

    //エフェクト間の中間描画先(交互に使う)
    std::shared_ptr<RenderTarget> workRenderTargets[2];

    //エフェクト(適用順)
    std::vector<EffectEntry> entries;

    //シーン描画パス
    std::shared_ptr<RenderPass> scenePass;
};
