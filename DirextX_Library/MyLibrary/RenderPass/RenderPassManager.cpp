#include "RenderPassManager.h"
#include "RenderPassManager.h"

void RenderPassManager::AddPass(const std::shared_ptr<RenderPass>& pass)
{
    if (pass == nullptr)
    {
        return;
    }

    passes.push_back(pass);
}

void RenderPassManager::Execute()
{
    for (auto& pass : passes)
    {
        if (pass == nullptr)
        {
            continue;
        }

        pass->Execute();
    }
}

void RenderPassManager::Clear()
{
    passes.clear();
}