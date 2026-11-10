#include "RenderPass.h"
#include "../Engine/Engine.h"

void RenderPass::Execute()
{
    if (!enabled)
    {
        return;
    }

    // 独自Beginがあれば優先する（ShadowMapなどの非カラー描画先用）。
    if (beginFunction)
    {
        beginFunction();
    }
    else if (renderTarget != nullptr)
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

    if (endFunction)
    {
        endFunction();
    }
    else if (renderTarget != nullptr)
    {
        renderTarget->EndRender();
    }
}
