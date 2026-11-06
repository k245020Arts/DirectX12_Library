#pragma once
#include <vector>
#include <memory>
#include <cstring>
#include <type_traits>
#include "../ConstBuffer/ConstBuffer.h"
#include "../Engine/Engine.h"

//ポストエフェクトごとの定数バッファ用パラメータを管理するクラス
//
//  ・パラメータ構造体 T は エフェクトの種類ごとに自由に定義する
//    (HLSL の cbuffer と同じ並び・16byte単位にすること)
//  ・フレーム数分の ConstBuffer を持ち、値を変更したときだけ描画時にGPUへ書き込む
//
//新しいエフェクトでの使い方
//  struct MyParam { float intensity = 1.0f; float padding[3] = {}; };   // cbuffer MyParam : register(b0) に対応
//
//  PostEffectParam<MyParam> param;          // メンバーに持つ
//  param.Create();                          // Init() で1回
//  param.Edit().intensity = 0.5f;           // 値の変更(Setterの中などで)
//  param.GetBuffer()                        // Draw() で取得して RootSignature の CBV に設定する
template<class T>
class PostEffectParam
{
    static_assert(std::is_trivially_copyable<T>::value, "T は memcpy できる構造体にしてください");
    static_assert(sizeof(T) % 16 == 0, "T のサイズは16byteの倍数にしてください(cbufferのアライメント)");

public:

    //フレーム数分の定数バッファを生成
    bool Create()
    {
        buffers.clear();

        for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
        {
            auto buffer = std::make_unique<ConstBuffer>(sizeof(T));

            if (!buffer->IsValid())
            {
                buffers.clear();
                return false;
            }

            buffers.push_back(std::move(buffer));
        }

        //全フレーム分を未反映にする
        dirty.assign(buffers.size(), true);

        return true;
    }

    //値をまとめて設定
    void Set(const T& _value)
    {
        value = _value;
        MarkDirty();
    }

    //値を直接書き換えたいとき用(変更扱いになる)
    T& Edit()
    {
        MarkDirty();
        return value;
    }

    const T& Get() const
    {
        return value;
    }

    //現在のフレーム用の定数バッファを返す(未反映の変更があればここで書き込む)
    //Create()前は nullptr
    ConstBuffer* GetBuffer()
    {
        if (buffers.empty())
        {
            return nullptr;
        }

        const size_t index = Engine::GetInstance()->CurrentBackBufferIndex() % buffers.size();

        if (dirty[index])
        {
            std::memcpy(buffers[index]->GetPtr(), &value, sizeof(T));

            dirty[index] = false;
        }

        return buffers[index].get();
    }

private:

    void MarkDirty()
    {
        dirty.assign(dirty.size(), true);
    }

    T value{};

    std::vector<std::unique_ptr<ConstBuffer>> buffers;

    std::vector<bool> dirty;
};
