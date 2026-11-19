#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <utility>
#include "../ShaderStruct/ShaderStruct.h"

class RootSignature;
class PipelineState;
struct DescriptorHandle;
class ConstBuffer;
class RenderTarget;

//ポストエフェクトが必要とする描画パス1つ分の情報
struct PostEffectPassInfo
{
    //このパスで実行する描画処理
    std::function<void()> execute;

    //描画先
    //  ・中間バッファが必要なパス(ブラーの横方向など)は、自分で持っているRenderTargetを指定する
    //  ・nullptr のパスが「このエフェクトの最終出力パス」になり、
    //    出力先(次エフェクトの入力用RT or BackBuffer)はPostEffectPipelineが自動で決める
    RenderTarget* target = nullptr;
};

//シーン描画(マルチレンダーターゲット)で書かれたテクスチャ一覧
//PostEffectPipeline が用意して、各エフェクトに渡す(使わないエフェクトは無視してよい)
//  ターゲットを増やしたとき、または被写界深度のように深度が必要になったときは、ここに足す
struct SceneTextures
{
    DescriptorHandle* color = nullptr; //通常の描画結果(SV_TARGET0)
    DescriptorHandle* bloom = nullptr; //光らせたい色(SV_TARGET1)
    DescriptorHandle* depth = nullptr; //シーンの深度(R32_FLOAT)。NeedsSceneDepth() が true のエフェクトが有効な間だけ入る
};

class PostEffect
{
public:

    PostEffect();

    virtual ~PostEffect();

    virtual bool Init() = 0;

    virtual void Draw() = 0;

    //このエフェクトが必要とするパスを実行順に返す
    //ルール: 最後のパスだけ target = nullptr (最終出力)、それ以外は中間RenderTargetを指定する
    //  例) 1パスで済むエフェクト : { { [this]{ Draw(); }, nullptr } }
    //  例) 2パス必要なエフェクト : { { 横方向, 中間RT }, { 縦方向, nullptr } }
    virtual std::vector<PostEffectPassInfo> CreatePasses() = 0;

    void SetInputTexture(DescriptorHandle* handle);

    //このエフェクトがシーンの深度(SceneTextures::depth)を使うかどうか
    //true のエフェクトが有効な間だけ、PostEffectPipelineがシーン描画後に深度をコピーして渡す
    virtual bool NeedsSceneDepth() const
    {
        return false;
    }

    //シーン描画で書かれたテクスチャ(Bloom など)を使うエフェクト用
    //PostEffectPipelineに追加済みなら、自動で設定される
    void SetSceneTextures(const SceneTextures* _textures)
    {
        sceneTextures = _textures;
    }

    //このエフェクトの有効/無効(初期値は有効)
    //無効の間はこのエフェクトのパスは実行されず、前段の結果がそのまま次へ渡る
    //PostEffectPipelineに追加済みなら、入出力のつなぎ直しは自動で行われる
    void SetEnabled(bool _enabled)
    {
        if (enabled == _enabled)
        {
            return;
        }

        enabled = _enabled;

        if (onStateChanged)
        {
            onStateChanged();
        }
    }

    bool IsEnabled() const
    {
        return enabled;
    }

    //状態変更の通知先(PostEffectPipelineが内部で設定する)
    void SetStateChangedCallback(std::function<void()> _callback)
    {
        onStateChanged = std::move(_callback);
    }

protected:
    std::unique_ptr<RootSignature> rootSignature = nullptr;

    std::unique_ptr<PipelineState> pipelineState = nullptr;

    DescriptorHandle* inputTexture = nullptr;

    const SceneTextures* sceneTextures = nullptr;

    void DrawFullscreen(DescriptorHandle* inputTexture,ConstBuffer* constBuffer);

private:

    bool enabled = true;

    std::function<void()> onStateChanged;

};

