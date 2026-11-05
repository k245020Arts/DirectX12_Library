#pragma once
#include <functional>
#include <memory>
#include "../RenderTarget/RenderTarget.h"
#include "../ShaderStruct/ShaderStruct.h"

class RenderPass
{
public:

    void SetRenderTarget(RenderTarget* target)
    {
        renderTarget = target;
    }

    void SetClearColor(const Vector4& color)
    {
        clearColor = color;
    }

    void SetExecuteFunction(std::function<void()> function)
    {
        executeFunction = function;
    }

    void Execute();

private:

    RenderTarget* renderTarget = nullptr;

    Vector4 clearColor = {0.25f,0.25f,0.25f, 1.0f };

    std::function<void()> executeFunction;
};

