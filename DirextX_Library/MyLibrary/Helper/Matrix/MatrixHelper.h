#pragma once

#include <DirectXMath.h>
#include "../Vector/VectorHelper.h"

class MatrixHelper
{
public:

    class Matrix : public DirectX::XMMATRIX
    {
    public:
        using Base = DirectX::XMMATRIX;

        // コンストラクタ
        Matrix()
            : Base(DirectX::XMMatrixIdentity())
        {
        }

        Matrix(const Base& matrix)
            : Base(matrix)
        {
        }

        Matrix(const Matrix& matrix)
            : Base(static_cast<const Base&>(matrix))
        {
        }

        // 行列の加算
        Matrix operator+(const Matrix& rhs) const
        {
            DirectX::XMMATRIX result;

            for (int i = 0; i < 4; ++i)
            {
                result.r[i] = DirectX::XMVectorAdd(
                    this->r[i],
                    rhs.r[i]
                );
            }

            return Matrix(result);
        }

        // 行列の減算
        Matrix operator-(const Matrix& rhs) const
        {
            DirectX::XMMATRIX result;

            for (int i = 0; i < 4; ++i)
            {
                result.r[i] = DirectX::XMVectorSubtract(
                    this->r[i],
                    rhs.r[i]
                );
            }

            return Matrix(result);
        }

        // 行列の乗算
        Matrix operator*(const Matrix& rhs) const
        {
            return Matrix(
                DirectX::XMMatrixMultiply(*this, rhs)
            );
        }

        // スカラー乗算
        Matrix operator*(float scalar) const
        {
            return Matrix(
                DirectX::XMMatrixMultiply(
                    *this,
                    DirectX::XMMatrixScaling(
                        scalar, scalar, scalar
                    )
                )
            );
        }

        // スカラー除算
        Matrix operator/(float scalar) const
        {
            if (scalar == 0.0f)
            {
                return Matrix();
            }

            return *this * (1.0f / scalar);
        }

        // 加算代入
        Matrix& operator+=(const Matrix& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        // 減算代入
        Matrix& operator-=(const Matrix& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        // 乗算代入
        Matrix& operator*=(const Matrix& rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        // スカラー乗算代入
        Matrix& operator*=(float scalar)
        {
            *this = *this * scalar;
            return *this;
        }

        // スカラー除算代入
        Matrix& operator/=(float scalar)
        {
            *this = *this / scalar;
            return *this;
        }

        // 転置
        Matrix Transpose() const
        {
            return Matrix(
                DirectX::XMMatrixTranspose(*this)
            );
        }

        // 逆行列
        Matrix Inverse() const
        {
            return Matrix(
                DirectX::XMMatrixInverse(nullptr, *this)
            );
        }

        // 単位行列
        static Matrix Identity()
        {
            return Matrix(DirectX::XMMatrixIdentity());
        }

        // 平行移動
        static Matrix Translation(
            const VectorHelper::Vector3& position)
        {
            return Matrix(
                DirectX::XMMatrixTranslation(
                    position.x,
                    position.y,
                    position.z
                )
            );
        }

        // 拡大縮小
        static Matrix Scale(
            const VectorHelper::Vector3& scale)
        {
            return Matrix(
                DirectX::XMMatrixScaling(
                    scale.x,
                    scale.y,
                    scale.z
                )
            );
        }

        // XYZ回転（ラジアン）
        static Matrix Rotation(
            const VectorHelper::Vector3& rotation)
        {
            return Matrix(
                DirectX::XMMatrixRotationRollPitchYaw(
                    rotation.x,
                    rotation.y,
                    rotation.z
                )
            );
        }

        // 左手座標系のビュー行列
        static Matrix LookAtLH(const VectorHelper::Vector3& eye,const VectorHelper::Vector3& target,const VectorHelper::Vector3& up)
        {
            using namespace DirectX;

            XMVECTOR e = XMVectorSet(eye.x, eye.y, eye.z, 1.0f);

            XMVECTOR t = XMVectorSet(target.x, target.y, target.z, 1.0f);

            XMVECTOR u = XMVectorSet( up.x, up.y, up.z, 0.0f);

            return Matrix(XMMatrixLookAtLH(e, t, u));
        }

        // 左手座標系の透視投影
        static Matrix PerspectiveFovLH(float fovY,float aspectRatio,float nearZ,float farZ)
        {
            return Matrix(DirectX::XMMatrixPerspectiveFovLH(fovY,aspectRatio,nearZ,farZ
                )
            );
        }

        // 位置ベクトルを変換（平行移動の影響あり）
        VectorHelper::Vector3 TransformPoint(const VectorHelper::Vector3& point) const
        {
            DirectX::XMVECTOR v = DirectX::XMVectorSet(
                point.x, point.y, point.z, 1.0f
            );

            v = DirectX::XMVector3TransformCoord(v, *this);

            DirectX::XMFLOAT3 result;
            DirectX::XMStoreFloat3(&result, v);

            return VectorHelper::Vector3(
                result.x, result.y, result.z
            );
        }

        // 方向ベクトルを変換（平行移動の影響なし）
        VectorHelper::Vector3 TransformDirection(
            const VectorHelper::Vector3& direction) const
        {
            DirectX::XMVECTOR v = DirectX::XMVectorSet(
                direction.x, direction.y, direction.z, 0.0f
            );

            v = DirectX::XMVector3TransformNormal(v, *this);

            DirectX::XMFLOAT3 result;
            DirectX::XMStoreFloat3(&result, v);

            return VectorHelper::Vector3(
                result.x, result.y, result.z
            );
        }
    };
};

using Matrix = MatrixHelper::Matrix;

// 左側からのスカラー乗算
inline Matrix operator*(float scalar, const Matrix& matrix)
{
    return matrix * scalar;
}