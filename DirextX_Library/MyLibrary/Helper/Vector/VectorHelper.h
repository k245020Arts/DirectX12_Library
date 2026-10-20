#pragma once
#include <DirectXMath.h>
#include <cmath>

using VEC2 = DirectX::XMFLOAT2;
using VEC3 = DirectX::XMFLOAT3;
using VEC4 = DirectX::XMFLOAT4;

inline constexpr float DegToRad = DirectX::XM_PI / 180.0f;

inline constexpr float RadToDeg = 180.0f / DirectX::XM_PI;

class VectorHelper
{
public:
    class Vector3 : public VEC3
    {
    public:
        Vector3() : VEC3(0.0f, 0.0f, 0.0f) {}

        Vector3(float x, float y, float z)
            : VEC3(x, y, z) {
        }

        explicit Vector3(float value)
            : VEC3(value, value, value) {
        }

        Vector3(const VEC3& v)
            : VEC3(v.x, v.y, v.z) {
        }

        // 加算
        Vector3 operator+(const Vector3& v) const
        {
            return VectorHelper::VAdd(*this, v);
        }

        Vector3& operator+=(const Vector3& v)
        {
            *this = VectorHelper::VAdd(*this, v);
            return *this;
        }

        // 減算
        Vector3 operator-(const Vector3& v) const
        {
            return VectorHelper::VSub(*this, v);
        }

        Vector3& operator-=(const Vector3& v)
        {
            *this = VectorHelper::VSub(*this, v);
            return *this;
        }

        // スカラー倍
        Vector3 operator*(float scale) const
        {
            return VectorHelper::VScale(*this, scale);
        }

        Vector3& operator*=(float scale)
        {
            *this = VectorHelper::VScale(*this, scale);
            return *this;
        }

        Vector3 operator/(float scale) const
        {
            if (scale == 0.0f)
                return Vector3();

            return VectorHelper::VScale(*this, 1.0f / scale);
        }

        Vector3& operator/=(float scale)
        {
            if (scale != 0.0f)
                *this = VectorHelper::VScale(*this, 1.0f / scale);

            return *this;
        }

        // 単項マイナス
        Vector3 operator-() const
        {
            return Vector3(-x, -y, -z);
        }

        // 成分ごとの乗算
        Vector3 operator*(const Vector3& v) const
        {
            return Vector3(x * v.x, y * v.y, z * v.z);
        }

        Vector3& operator*=(const Vector3& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        bool operator==(const Vector3& v) const
        {
            return x == v.x && y == v.y && z == v.z;
        }

        bool operator!=(const Vector3& v) const
        {
            return !(*this == v);
        }

        // 長さの二乗
        float SquareSize() const
        {
            return VectorHelper::VDot(*this, *this);
        }

        // 長さ
        float Size() const
        {
            return std::sqrt(SquareSize());
        }

        // 正規化
        Vector3 Normalize() const
        {
            const float size = Size();

            if (size <= 0.000001f)
                return Vector3();

            return *this / size;
        }

        // 内積
        float Dot(const Vector3& v) const
        {
            return VectorHelper::VDot(*this, v);
        }

        // 外積
        Vector3 Cross(const Vector3& v) const
        {
            return VectorHelper::VCross(*this, v);
        }

        // 2点間の距離
        float Distance(const Vector3& v) const
        {
            return (*this - v).Size();
        }
    };

    // 左辺がスカラーの乗算にも対応
    friend Vector3 operator*(float scale, const Vector3& v)
    {
        return v * scale;
    }

    // 定数
    inline static const Vector3 VZero = Vector3(0.0f);
    inline static const Vector3 VOne = Vector3(1.0f);

    // ベクトルの加算
    static Vector3 VAdd(const VEC3& a, const VEC3& b)
    {
        return Vector3(
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        );
    }

    // ベクトルの減算
    static Vector3 VSub(const VEC3& a, const VEC3& b)
    {
        return Vector3(
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
        );
    }

    // スカラー倍
    static Vector3 VScale(const VEC3& v, float scale)
    {
        return Vector3(
            v.x * scale,
            v.y * scale,
            v.z * scale
        );
    }

    // 内積
    static float VDot(const VEC3& a, const VEC3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // 外積
    static Vector3 VCross(const VEC3& a, const VEC3& b)
    {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }


    template<typename T>
    class Vector2
    {
    public:
        T x = T{};
        T y = T{};

        Vector2() = default;

        explicit Vector2(T value)
            : x(value), y(value) {
        }

        Vector2(T x, T y)
            : x(x), y(y) {
        }

        // 別の型のVector2から変換
        template<typename U>
        explicit Vector2(const Vector2<U>& v)
            : x(static_cast<T>(v.x)),
            y(static_cast<T>(v.y)) {
        }

        // 加算
        Vector2 operator+(const Vector2& v) const
        {
            return V2Add(*this, v);
        }

        Vector2& operator+=(const Vector2& v)
        {
            *this = *this + v;
            return *this;
        }

        // 減算
        Vector2 operator-(const Vector2& v) const
        {
            return V2Sub(*this, v);
        }

        Vector2& operator-=(const Vector2& v)
        {
            *this = *this - v;
            return *this;
        }

        // スカラー倍
        Vector2 operator*(T scale) const
        {
            return V2Scale(*this, scale);
        }

        Vector2& operator*=(T scale)
        {
            *this = *this * scale;
            return *this;
        }

        // スカラー除算
        Vector2 operator/(T scale) const
        {
            if (scale == T{})
                return Vector2();

            return V2Scale(*this, T(1) / scale);
        }

        Vector2& operator/=(T scale)
        {
            if (scale != T{})
                *this = *this / scale;

            return *this;
        }

        // 単項マイナス
        Vector2 operator-() const
        {
            return Vector2(-x, -y);
        }

        // 等値比較
        bool operator==(const Vector2& v) const
        {
            return x == v.x && y == v.y;
        }

        bool operator!=(const Vector2& v) const
        {
            return !(*this == v);
        }

        // 長さの二乗
        T SquareSize() const
        {
            return V2Dot(*this, *this);
        }

        // 長さ
        float Size() const
        {
            return std::sqrt(
                static_cast<float>(SquareSize()));
        }

        // 正規化
        Vector2 Normalize() const
        {
            const float size = Size();

            if (size <= 0.000001f)
                return Vector2();

            return Vector2(
                static_cast<T>(x / size),
                static_cast<T>(y / size));
        }

        // 内積
        T Dot(const Vector2& v) const
        {
            return VectorHelper::V2Dot(*this, v);
        }

        // 2点間の距離
        float Distance(const Vector2& v) const
        {
            return (*this - v).Size();
        }

        // 原点を中心に回転（ラジアン）
        Vector2 Rotate(float angle) const
        {
            const float c = std::cos(angle);
            const float s = std::sin(angle);

            return Vector2(
                static_cast<T>(x * c - y * s),
                static_cast<T>(x * s + y * c));
        }

        // 左辺がスカラーの乗算
        friend Vector2 operator*(T scale, const Vector2& v)
        {
            return v * scale;
        }
        template<typename T>
        static Vector2<T> V2Add(
            const Vector2<T>& a,
            const Vector2<T>& b)
        {
        return Vector2<T>(
            a.x + b.x,
            a.y + b.y);
        }

        template<typename T>
        static Vector2<T> V2Sub(
            const Vector2<T>& a,
            const Vector2<T>& b)
        {
            return Vector2<T>(
                a.x - b.x,
                a.y - b.y);
         }

        template<typename T>
        static Vector2<T> V2Scale(
            const Vector2<T>& v,
            T scale)
        {
            return Vector2<T>(
                v.x * scale,
                v.y * scale);
        }

        template<typename T>
        static T V2Dot(const Vector2<T>& a,const Vector2<T>& b)
        {
            return a.x * b.x + a.y * b.y;
        }
    };

    class Vector4 : public VEC4
    {
    public:
        Vector4()
            : VEC4(0.0f, 0.0f, 0.0f, 0.0f) {
        }

        explicit Vector4(float value)
            : VEC4(value, value, value, value) {
        }

        Vector4(float x, float y, float z, float w)
            : VEC4(x, y, z, w) {
        }

        Vector4(const VEC4& v)
            : VEC4(v.x, v.y, v.z, v.w) {
        }

        // 加算
        Vector4 operator+(const Vector4& v) const
        {
            return VectorHelper::V4Add(*this, v);
        }

        Vector4& operator+=(const Vector4& v)
        {
            *this = *this + v;
            return *this;
        }

        // 減算
        Vector4 operator-(const Vector4& v) const
        {
            return VectorHelper::V4Sub(*this, v);
        }

        Vector4& operator-=(const Vector4& v)
        {
            *this = *this - v;
            return *this;
        }

        // スカラー倍
        Vector4 operator*(float scale) const
        {
            return VectorHelper::V4Scale(*this, scale);
        }

        Vector4& operator*=(float scale)
        {
            *this = *this * scale;
            return *this;
        }

        // スカラー除算
        Vector4 operator/(float scale) const
        {
            if (scale == 0.0f)
                return Vector4();

            return VectorHelper::V4Scale(*this, 1.0f / scale);
        }

        Vector4& operator/=(float scale)
        {
            if (scale != 0.0f)
                *this = *this / scale;

            return *this;
        }

        // 単項マイナス
        Vector4 operator-() const
        {
            return Vector4(-x, -y, -z, -w);
        }

        // 成分ごとの乗算
        Vector4 operator*(const Vector4& v) const
        {
            return Vector4(
                x * v.x,
                y * v.y,
                z * v.z,
                w * v.w);
        }

        Vector4& operator*=(const Vector4& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            w *= v.w;
            return *this;
        }

        bool operator==(const Vector4& v) const
        {
            return x == v.x && y == v.y &&
                z == v.z && w == v.w;
        }

        bool operator!=(const Vector4& v) const
        {
            return !(*this == v);
        }

        // 長さの二乗
        float SquareSize() const
        {
            return VectorHelper::V4Dot(*this, *this);
        }

        // 長さ
        float Size() const
        {
            return std::sqrt(SquareSize());
        }

        // 正規化
        Vector4 Normalize() const
        {
            const float size = Size();

            if (size <= 0.000001f)
                return Vector4();

            return *this / size;
        }

        // 内積
        float Dot(const Vector4& v) const
        {
            return VectorHelper::V4Dot(*this, v);
        }

        // 4次元ベクトル間の距離
        float Distance(const Vector4& v) const
        {
            return (*this - v).Size();
        }

        friend Vector4 operator*(float scale, const Vector4& v)
        {
            return v * scale;
        }
    };

    static Vector4 V4Add(
        const VEC4& a,
        const VEC4& b)
    {
        return Vector4(
            a.x + b.x,
            a.y + b.y,
            a.z + b.z,
            a.w + b.w);
    }

    static Vector4 V4Sub(
        const VEC4& a,
        const VEC4& b)
    {
        return Vector4(
            a.x - b.x,
            a.y - b.y,
            a.z - b.z,
            a.w - b.w);
    }

    static Vector4 V4Scale(
        const VEC4& v,
        float scale)
    {
        return Vector4(
            v.x * scale,
            v.y * scale,
            v.z * scale,
            v.w * scale);
    }

    static float V4Dot(
        const VEC4& a,
        const VEC4& b)
    {
        return a.x * b.x + a.y * b.y +
            a.z * b.z + a.w * b.w;
    }


private:
    
};

using Vector2 = VectorHelper::Vector2<float>;
using Vector2i = VectorHelper::Vector2<int>;

using Vector3 = VectorHelper::Vector3;
using Vector4 = VectorHelper::Vector4;