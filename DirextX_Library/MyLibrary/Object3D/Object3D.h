#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "Object3DManager.h"

class Camera3D;
class ShadowCamera;
class ShadowMap;

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

    std::string GetTag() { return tag;; }
    void SetTag(std::string& _tag) { tag = _tag; }

protected:
    Transform transform;

    virtual void Update() {};
    virtual void Draw() {};

    // シャドウマップ用の描画
    virtual void DrawShadow(const ShadowCamera& _shadowCamera, const ShadowMap& _shadowMap) {};

    virtual void VertexMapping(const Camera3D* _camera) { ; }

    virtual void ShaderParamSerializeImgui() {};

    friend class Object3DManager;

    std::string tag;

private:
    bool isDestory;
    int drawOrder;
};