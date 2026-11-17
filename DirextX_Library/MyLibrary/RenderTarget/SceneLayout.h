#pragma once
#include <dxgiformat.h>
#include "../ShaderStruct/ShaderStruct.h"

// ============================================================================
//  シーン描画(3Dオブジェクトを描くパス)のレンダーターゲット構成
//
//  1回の描画で、下の表のターゲットへ同時に書き込む(マルチレンダーターゲット)
//
//    SV_TARGET0 : Color  通常の描画結果
//    SV_TARGET1 : Bloom  光らせたい部分(ブルームの元)
//
//  ターゲットを増やすとき(例: 被写界深度で使う CoC やマスク)
//    1. Slot に名前を足す(Count の手前)
//    2. Formats / ClearColors に同じ順番で足す
//    3. 出力するシェーダーの ScenePSOutput に SV_TARGETn を足す
//    ※ このレイアウトで描くパイプラインステートは、必ず同じフォーマット並びで作ること
//      (PipelineState::SetRenderTargetFormats(SceneLayout::Formats, SceneLayout::Count))
// ============================================================================
namespace SceneLayout
{
    enum Slot : unsigned int
    {
        Color = 0,
        Bloom = 1,

        Count // ターゲットの数
    };

    inline constexpr DXGI_FORMAT Formats[Count] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM, // Color
        DXGI_FORMAT_R8G8B8A8_UNORM, // Bloom (HDR にするなら R16G16B16A16_FLOAT)
    };

    inline const Vector4 ClearColors[Count] =
    {
        Vector4(0.25f, 0.25f, 0.25f, 1.0f), // Color
        Vector4(0.0f, 0.0f, 0.0f, 0.0f),    // Bloom (何も光らない状態)
    };
}
