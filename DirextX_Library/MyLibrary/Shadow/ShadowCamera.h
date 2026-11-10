#pragma once

#include "../Helper/Helper.h"

// ディレクショナルライトから見た深度を書き込むための直交投影カメラ。
class ShadowCamera
{
public:
    ShadowCamera() = default;

    // LightData::direction（光が進む向き）からカメラ行列を更新する。
    void Update(const Vector3& lightDirection);

    void SetTarget(const Vector3& value) { target = value; }
    void SetDistance(float value);
    void SetOrthographicSize(float width, float height);
    void SetNearFar(float nearValue, float farValue);

    const Vector3& GetPosition() const { return position; }
    const Vector3& GetTarget() const { return target; }
    const Matrix& GetViewMatrix() const { return viewMatrix; }
    const Matrix& GetProjectionMatrix() const { return projectionMatrix; }

    float GetDistance() const { return distance; }
    float GetOrthographicWidth() const { return orthographicWidth; }
    float GetOrthographicHeight() const { return orthographicHeight; }

private:
    Vector3 position{ -300.0f, 300.0f, -300.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };
    Vector3 up{ 0.0f, 1.0f, 0.0f };

    Matrix viewMatrix{};
    Matrix projectionMatrix{};

    float distance = 500.0f;
    float orthographicWidth = 1000.0f;
    float orthographicHeight = 1000.0f;
    float nearZ = 1.0f;
    float farZ = 1500.0f;
};
