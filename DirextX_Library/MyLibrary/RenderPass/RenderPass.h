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

    //–³Œø‚ÌŠÔ‚Í Execute() ‚Å‰½‚às‚í‚È‚¢(RenderTarget‚ÌØ‚è‘Ö‚¦‚às‚í‚È‚¢)
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

    std::function<void()> executeFunction;
};

