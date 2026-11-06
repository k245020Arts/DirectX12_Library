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

    void DrawFullscreen(DescriptorHandle* inputTexture,ConstBuffer* constBuffer);

private:

    bool enabled = true;

    std::function<void()> onStateChanged;

};

