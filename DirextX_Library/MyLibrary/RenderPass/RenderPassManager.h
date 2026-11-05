#pragma once
#include <vector>
#include <memory>

#include "../SingleTon/SingletonBase.h"
#include "RenderPass.h"

class RenderPassManager : public SingletonBase<RenderPassManager>
{
public:

    void AddPass(const std::shared_ptr<RenderPass>& pass);

    void Execute();

    void Clear();

private:

    std::vector<std::shared_ptr<RenderPass>> passes;

    friend class SingletonBase<RenderPassManager>;
};

