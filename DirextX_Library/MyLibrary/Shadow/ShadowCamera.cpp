#include "ShadowCamera.h"
#include <algorithm>
#include <cmath>

void ShadowCamera::Update()
{
    using namespace DirectX;

    Vector3 direction = lightDirection.Normalize();
    if (direction.SquareSize() < 0.000001f)
    {
        direction = Vector3(0.0f, -1.0f, 0.0f);
    }

    Vector3 cameraUp = up;
    if (std::fabs(direction.Dot(cameraUp)) > 0.99f)
    {
        cameraUp = Vector3(0.0f, 0.0f, 1.0f);
    }

    // ---- テクセルスナップ: targetをライト空間のテクセル幅の倍数に丸める ----
    Vector3 snappedTarget = target;
    if (texelSnap && mapWidth > 0 && mapHeight > 0)
    {
        const XMVECTOR f = XMVectorSet(direction.x, direction.y, direction.z, 0.0f);
        const XMVECTOR u = XMVectorSet(cameraUp.x, cameraUp.y, cameraUp.z, 0.0f);
        const XMVECTOR r = XMVector3Normalize(XMVector3Cross(u, f)); // LookAtLHのx軸
        const XMVECTOR v = XMVector3Cross(f, r);                     // y軸
        const XMVECTOR t = XMVectorSet(target.x, target.y, target.z, 0.0f);

        const float texelX = orthographicWidth / static_cast<float>(mapWidth);
        const float texelY = orthographicHeight / static_cast<float>(mapHeight);

        float x = XMVectorGetX(XMVector3Dot(t, r));
        float y = XMVectorGetX(XMVector3Dot(t, v));
        const float z = XMVectorGetX(XMVector3Dot(t, f));

        x = std::floor(x / texelX) * texelX;
        y = std::floor(y / texelY) * texelY;

        XMVECTOR s = XMVectorScale(r, x);
        s = XMVectorAdd(s, XMVectorScale(v, y));
        s = XMVectorAdd(s, XMVectorScale(f, z));
        snappedTarget = Vector3(XMVectorGetX(s), XMVectorGetY(s), XMVectorGetZ(s));
    }

    position = snappedTarget - direction * distance;

    const auto eye = XMVectorSet(position.x, position.y, position.z, 1.0f);
    const auto lookAt = XMVectorSet(snappedTarget.x, snappedTarget.y, snappedTarget.z, 1.0f);
    const auto upVector = XMVectorSet(cameraUp.x, cameraUp.y, cameraUp.z, 0.0f);

    viewMatrix = XMMatrixLookAtLH(eye, lookAt, upVector);
    projectionMatrix = XMMatrixOrthographicLH(orthographicWidth, orthographicHeight, nearZ, farZ);
}

void ShadowCamera::SetDistance(float value)
{
    distance = (std::max)(value, nearZ + 0.01f);
}

void ShadowCamera::SetOrthographicSize(float width, float height)
{
    orthographicWidth = (std::max)(width, 0.01f);
    orthographicHeight = (std::max)(height, 0.01f);
}

void ShadowCamera::SetNearFar(float nearValue, float farValue)
{
    nearZ = (std::max)(nearValue, 0.01f);
    farZ = (std::max)(farValue, nearZ + 0.01f);

    if (distance <= nearZ)
    {
        distance = nearZ + 0.01f;
    }
}

void ShadowCamera::SetDirection(const Vector3& _direction)
{
    lightDirection = _direction;
}
