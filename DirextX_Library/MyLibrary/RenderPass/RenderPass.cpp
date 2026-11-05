#include "RenderPass.h"

#include "../Engine/Engine.h"

void RenderPass::Execute()
{
    if (renderTarget != nullptr)
    {
        renderTarget->BeginRender(clearColor);
    }
    else
    {
        Engine::GetInstance()->BindBackBuffer(clearColor);
    }

    if (executeFunction)
    {
        executeFunction();
    }

    if (renderTarget != nullptr)
    {
        renderTarget->EndRender();
    }
}