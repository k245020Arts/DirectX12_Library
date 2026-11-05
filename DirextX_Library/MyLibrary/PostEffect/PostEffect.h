#pragma once
#include <memory>
#include "../ShaderStruct/ShaderStruct.h"

class RootSignature;
class PipelineState;
struct DescriptorHandle;
class ConstBuffer;

class PostEffect
{
public:

    PostEffect();

    virtual ~PostEffect();

    virtual bool Init() = 0;

    virtual void Draw() = 0;

    void SetInputTexture(DescriptorHandle* handle);

protected:
    std::unique_ptr<RootSignature> rootSignature = nullptr;

    std::unique_ptr<PipelineState> pipelineState = nullptr;

    DescriptorHandle* inputTexture = nullptr;

    void DrawFullscreen(DescriptorHandle* inputTexture,ConstBuffer* constBuffer);

};

