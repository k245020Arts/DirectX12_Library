#pragma once
#include <functional>
#include <DirectXMath.h>
#include "../Time/DeltaTime.h"
/// <summary>
/// easingを使うときはこの関数を使う
/// </summary>
/// 


/// <typeparam name="T">Easingによる取得をする型</typeparam>
/// <param name="a">Easingの最小（tが0になった時の値)</param>
/// <param name="b">Easingの最小（tが1になった時の値)</param>
/// <param name="t">0～1割合</param>
/// <returns>各々Easingによって返される値</returns>
namespace Easing {
    /// <summary>
    /// 二点の間の値を求める
    /// 等速
    /// </summary>
    template<typename T>
    T inline Lerp(const T a, const T b, const float t) {
        return static_cast<T>(a + (b - a) * t);
    }
    /// <summary>
    /// 徐々に加速するLerp
    /// </summary>
    template<typename T>
    T inline EaseIn(const T a, const T b, float t) {
        t = t * t;
        return static_cast<T>(Lerp<T>(a,b,t));
    }

    /// <summary>
    /// 徐々に停止するLerp
    /// </summary>
    template<typename T>
    T inline EaseOut(const T a, const T b, float t) {
        t = 1 - (1 - t) * (1 - t);
        return static_cast<T>(Lerp<T>(a, b, t));
    }

    /// <summary>
    /// スムーズに加速し終わり際に減速
    /// </summary>
    template<typename T>
    T inline EaseInOut(const T a, const T b, float t) {
        if (t < 0.5f) {
            t = 2 * t * t;
        }
        else {
            t = 1 - ((t - 1) * (t - 1) * 2);
        }
        return static_cast<T>(Lerp<T>(a, b, t));
    }

    /// <summary>
    /// よりスムーズに変化
    /// </summary>
    template<typename T>
    T inline SmoothStep(const T a, const T b, float t) {
        t = t * t * (3 - 2 * t);
        return static_cast<T>(Lerp<T>(a, b, t));
    }

    /// <summary>
    /// 急激に上がって少し戻るようなイージング
    /// </summary>
    
    template<typename T>
    T inline EaseInExpo(const T a, const  T b, float t) {
        t = powf(2, 10 * t - 10);
        return static_cast<T>(Lerp<T>(a, b, t));
    }



    /// <summary>
    /// 逆方向に少し戻ってから最終位置で減速するイージング
    /// </summary>
    template<typename T>
    T inline EaseOutBack(const T a, const T b, float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;

        t = 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
        return static_cast<T>(Lerp<T>(a, b, t));
    }
    /// <summary>
    /// 弾むような振動で終了するイージング (Ease Out Elastic)
    /// </summary>
    template<typename T>
    T inline EaseOutElastic(const T a, const T b, const float t) {
        const float c4 = (2.0f * DirectX::XM_PI) / 3.0f;

        if (t == 0.0f) return a;
        if (t == 1.0f) return b;

        float easedT = powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
        return static_cast<T>(Lerp<T>(a, b, easedT));
    }
    /// <summary>
    /// 弾むような振動で開始するイージング (Ease In Elastic)
    /// </summary>
    template<typename T>
    T inline EaseInElastic(const T a, const T b, const float t) {
        const float c4 = (2.0f * DirectX::XM_PI) / 3.0f;

        if (t == 0.0f)
            return a;
        if (t == 1.0f)
            return b;

        float eased = -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);
        return static_cast<T>(Lerp<T>(a, b, eased));
    }
    /// <summary>
    /// 軽く戻しを伴いながら加速して開始するイージング (Ease In Back)
    /// </summary>
    template<typename T>
    T inline EaseInBack(const T a, const T b, float t) {

        const float c1 = 1.70158f;
        const float c3 = c1 + 1;

        t = c3 * t * t * t - c1 * t * t;
        return static_cast<T>(Lerp<T>(a, b, t));

    }
    /// <summary>
    /// サインカーブをする
    /// </summary>
    
    template<typename T>
    T inline SinCube(const T a, const T b, float t) {

        t = sinf(t * DirectX::XM_PI);
        return static_cast<T>(Lerp<T>(a, b, t));

    }

    //sin波の90度まで(0～1にいって0に戻る動き)を行うための関数。
    template<typename T>
    T inline Sin90Cube(const T a, const T b, float t) {

        t = sinf(t * DirectX::XM_PI / 2);
        return static_cast<T>(Lerp<T>(a, b, t));

    }
    /// <summary>
    /// Easingのカウンタを下げてそこからEasingの値を掛けて値を取得する流れを関数化
    /// </summary>
    /// <typeparam name="T">Easingによって返される値の型</typeparam>
    /// <param name="_counter">今現在のカウンターのポインタ</param>
    /// <param name="_maxTime">最大の時間</param>
    /// <param name="min">Easingの最小（counterが0になった時の値)</param>
    /// <param name="max">Easingの最大<counterが_maxTimeの時の値)</param>
    /// <param name="_func">関数</param>
    /// <returns>_funcによって取得した値</returns>
    template<typename T>
    T EasingFlow(float* _counter, float _maxTime, const T min, const T max, std::function<T(T, T, float)> _func) {
        //カウンターが0より下になったらminの値を返す
        if (*_counter <= 0.0f) {
            float rate = *_counter / _maxTime;
            return static_cast<T>(_func(min, max, rate));
        }
        *_counter = max(*_counter - DeltaTime::GetDeltaTimeMulTimeScale(), 0.0f);
        float rate = *_counter / _maxTime;
        return static_cast<T>(_func(min, max, rate));
    }

}