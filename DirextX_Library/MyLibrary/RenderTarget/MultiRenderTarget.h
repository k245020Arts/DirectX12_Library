#pragma once
#include <memory>
#include <vector>
#include <dxgiformat.h>
#include "RenderTarget.h"

// 複数のRenderTargetを束ねて、1回の描画でまとめて書き込めるようにするクラス(マルチレンダーターゲット)
//
//  ・中身は既存の RenderTarget の集まり。1枚ずつ SRV で読めるので、ポストエフェクトの入力にそのまま使える
//  ・すべて同じ大きさ。フォーマットは1枚ごとに変えられる
//
//使い方
//  MultiRenderTarget mrt;
//  mrt.Create(width, height, formats, clearColors, count);
//  mrt.BeginRender();                    // 全ターゲットを描画可能にしてバインド＆クリア
//  ... 描画(PSOは同じフォーマット並びで作っておく) ...
//  mrt.EndRender();                      // 全ターゲットを読み取り可能に戻す
//  mrt.GetTarget(1)->GetSRV();           // 2枚目をテクスチャとして使う
class MultiRenderTarget
{
public:

    MultiRenderTarget() {}
    ~MultiRenderTarget() {}

    // _count 枚分のターゲットを作る(_formats と _clearColors は _count 個ずつ)
    bool Create(UINT _width, UINT _height, const DXGI_FORMAT* _formats, const Vector4* _clearColors, UINT _count);

    // 全ターゲットを RENDER_TARGET にして、深度バッファと一緒にバインドし、クリアする
    void BeginRender();

    // 全ターゲットを PIXEL_SHADER_RESOURCE に戻し、ビューポートを元に戻す
    void EndRender();

    RenderTarget* GetTarget(size_t _index) const
    {
        return _index < targets.size() ? targets[_index].get() : nullptr;
    }

    size_t GetCount() const
    {
        return targets.size();
    }

private:

    std::vector<std::unique_ptr<RenderTarget>> targets;
};
