#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "Object3DManager.h"

class Camera3D;

class Object3D
{
public:

    Object3D() { Object3DManager::GetInstance()->Push(this); isDestory = false; drawOrder = 0; transform = Transform(); };
    virtual ~Object3D() { Object3DManager::GetInstance()->Pop(this); };

    Transform& GetTransform() { return transform; }

    void DestoryMe() { isDestory = true; }

    bool IsDestory() { return isDestory; }

    void SetDrawOrder(int order)
    {
        drawOrder = order;
    }

    inline int GetDrawOrder() const { return drawOrder; }

protected:
    Transform transform;

    virtual void Update() {};
    virtual void Draw() {};

    virtual void VertexMapping(const Camera3D* _camera) { ; }

    friend class Object3DManager;

private:
    bool isDestory;
    int drawOrder;
};