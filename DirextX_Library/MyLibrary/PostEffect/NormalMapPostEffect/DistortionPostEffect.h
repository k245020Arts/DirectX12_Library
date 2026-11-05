#pragma once
#include "../PostEffect.h"
#include "../../DescriptorHeap/DescriptorHeap.h"

class RenderTarget;

class DistortionPostEffect : public PostEffect
{
public:

    DistortionPostEffect();
    ~DistortionPostEffect();

    ResultErrorHandle Load(const std::string& _path);
    bool Init() override;

    void Draw() override;

private:
    DescriptorHandle* descriptorHandle = nullptr;
};

