#include "ShadowCamera.h"
#include <algorithm>
#include <cmath>

void ShadowCamera::Update()
{
    Vector3 direction = lightDirection.Normalize();

    // ゼロベクトルが渡された場合のフォールバック。
    if (direction.SquareSize() < 0.000001f)
    {
        direction = Vector3(0.0f, -1.0f, 0.0f);
    }

    // LightData::directionは光の進行方向として扱う。
    // カメラはその反対側に置き、targetを見る。
    position = target - direction * distance;

    Vector3 cameraUp = up;
    if (std::fabs(direction.Dot(cameraUp)) > 0.99f)
    {
        cameraUp = Vector3(0.0f, 0.0f, 1.0f);
    }

    const auto eye = DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f);
    const auto lookAt = DirectX::XMVectorSet(target.x, target.y, target.z, 1.0f);
    const auto upVector = DirectX::XMVectorSet(cameraUp.x, cameraUp.y, cameraUp.z, 0.0f);

    viewMatrix = DirectX::XMMatrixLookAtLH(eye, lookAt, upVector);
    projectionMatrix = DirectX::XMMatrixOrthographicLH(orthographicWidth,orthographicHeight,nearZ,farZ);
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
