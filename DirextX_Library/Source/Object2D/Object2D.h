#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "Object2DManager.h"

class Object2D
{
public:

    Object2D() { Object2DManager::GetInstance()->Push(this); isDestory = false; drawOrder = 0; }
    virtual ~Object2D() { Object2DManager::GetInstance()->Pop(this); };

    Transform& GetTransform() { return transform; }

    void DestoryMe() { isDestory = true; }

    bool IsDestory() { return isDestory; }

    void SetDrawOrder(int order)
    {
        drawOrder = order;
        Object2DManager::GetInstance()->SortByOrder();
    }

    inline int GetDrawOrder() const { return drawOrder; }

protected:
    Transform transform;

    bool isDestory;

    int drawOrder;

    virtual void Update() {};
    virtual void Draw() {};

    friend class Object2DManager;
};