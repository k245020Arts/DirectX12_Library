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

    void SetBeginFunction(std::function<void()> function)
    {
        beginFunction = function;
    }

    void SetExecuteFunction(std::function<void()> function)
    {
        executeFunction = function;
    }

    void SetEndFunction(std::function<void()> function)
    {
        endFunction = function;
    }

    //無効の間は Execute() で何も行わない(RenderTargetの切り替えも行わない)
    void SetEnabled(bool _enabled)
    {
        enabled = _enabled;
    }

    bool IsEnabled() const
    {
        return enabled;
    }

    void Execute();

private:

    bool enabled = true;

    RenderTarget* renderTarget = nullptr;

    Vector4 clearColor = {0.25f,0.25f,0.25f, 1.0f };

    std::function<void()> beginFunction;
    std::function<void()> executeFunction;
    std::function<void()> endFunction;
};

