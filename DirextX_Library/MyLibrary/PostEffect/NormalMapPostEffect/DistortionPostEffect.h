#pragma once
#include "../PostEffect.h"
#include "../PostEffectParam.h"
#include "../../DescriptorHeap/DescriptorHeap.h"

class RenderTarget;

struct DistortionParam
{
    //˜c‚Ý‚Ì‹­‚³(UV‚Ì‚¸‚ç‚µ—Ê)
    float strength = 0.1f;

    float padding[3] = {};
};

class DistortionPostEffect : public PostEffect
{
public:

    DistortionPostEffect();
    ~DistortionPostEffect();

    ResultErrorHandle Load(const std::string& _path);
    bool Init() override;

    void Draw() override;

    std::vector<PostEffectPassInfo> CreatePasses() override;

    //˜c‚Ý‚Ì‹­‚³
    void SetStrength(float _strength);
    float GetStrength() const { return param.Get().strength; }

private:
    DescriptorHandle* descriptorHandle = nullptr;

    PostEffectParam<DistortionParam> param;
};

