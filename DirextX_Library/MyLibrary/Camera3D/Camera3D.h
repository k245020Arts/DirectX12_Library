#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "../Engine/Engine.h"
#include "../SingleTon/SingletonBase.h"

class Camera3D : public SingletonBase<Camera3D>
{
public:
    
    void Update();

    void SetPosition(const Vector3& _position);

    void SetTarget(const Vector3& _target);

    void SetFov(float _fav);
    void SetNearFar(float _near, float _far);

    void SetUpVec(const Vector3& _upVec);

    const DirectX::XMMATRIX& GetViewMatrix() const { return viewMatrix; }
    const DirectX::XMMATRIX& GetProjectionMatrix() const { return projectionMatrix; }

    const Vector3 GetPosition()const { return position; }
    const Vector3 GetTarget()const { return target; }
    const Vector3 GetUpVec()const { return upVec; }

private:

    Camera3D();
    ~Camera3D();

    struct NearFar
    {
        float cameraToNear;
        float cameraToFar;

        NearFar() {
            cameraToNear = 0.0f;
            cameraToFar = 0.0f;
        }
    };

    Vector3 position{ 0.0f, 0.0f, -10.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };
    Vector3 upVec{ 0.0f, 1.0f, 0.0f };

    DirectX::XMMATRIX viewMatrix{};
    DirectX::XMMATRIX projectionMatrix{};

    float fovAngle = 0.0f;
    NearFar nearFar ={};

    friend class SingletonBase<Camera3D>;
};