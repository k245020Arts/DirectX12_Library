#pragma once
#include <memory>
#include "../ShaderStruct/ShaderStruct.h"

class RootSignature;
class PipelineState;
struct DescriptorHandle;
class ConstBuffer;

struct PostEffectParam
{
    float bkWeights[8];
};

class PostEffect
{
public:
    PostEffect();
    ~PostEffect();

    bool Init();

    void SetInputTexture(DescriptorHandle* handle);

    void Draw();

private:
    std::unique_ptr<RootSignature> rootSignature = nullptr;

    std::unique_ptr<PipelineState> pipelineState = nullptr;

    DescriptorHandle* inputTexture = nullptr;

    std::vector<float> GetGaussianWeights(size_t count, float s);

    std::vector<std::shared_ptr<ConstBuffer>> postEffectConstBuff;
};

