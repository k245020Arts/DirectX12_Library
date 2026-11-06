#include "PostEffectPipeline.h"
#include "../RenderPass/RenderPassManager.h"
#include "../Engine/Engine.h"

bool PostEffectPipeline::Init(UINT _width, UINT _height)
{
    if (isInitialized)
    {
        return false;
    }

    width = _width;
    height = _height;

    //シーン描画用RenderTarget
    sceneRenderTarget = std::make_shared<RenderTarget>();

    if (!sceneRenderTarget->Create(width, height, DXGI_FORMAT_R8G8B8A8_UNORM))
    {
        sceneRenderTarget.reset();
        return false;
    }

    isInitialized = true;

    return true;
}

void PostEffectPipeline::SetSceneFunction(std::function<void()> _function)
{
    if (isBuilt)
    {
        return;
    }

    sceneFunction = std::move(_function);
}

RenderTarget* PostEffectPipeline::GetWorkRenderTarget(size_t _index)
{
    auto& target = workRenderTargets[_index % 2];

    if (target == nullptr)
    {
        target = std::make_shared<RenderTarget>();

        if (!target->Create(width, height, DXGI_FORMAT_R8G8B8A8_UNORM))
        {
            target.reset();
            return nullptr;
        }
    }

    return target.get();
}

bool PostEffectPipeline::Build()
{
    if (!isInitialized || isBuilt)
    {
        return false;
    }

    if (!sceneFunction)
    {
        return false;
    }

    //========================================
    // 中間RenderTargetを先に確保
    // 全エフェクトが有効な場合に必要な枚数(最大2枚)。実行中に生成しないで済むようにする
    //========================================

    if (entries.size() >= 2)
    {
        const size_t workCount = (entries.size() - 1 < 2) ? entries.size() - 1 : 2;

        for (size_t i = 0; i < workCount; i++)
        {
            if (GetWorkRenderTarget(i) == nullptr)
            {
                OutputDebugStringW(L"PostEffectPipeline : 中間RenderTargetの生成に失敗\n");
                return false;
            }
        }
    }

    //========================================
    // 各エフェクトのパス情報を取得して検証
    // 途中で失敗しても RenderPassManager を汚さないよう、登録は最後にまとめて行う
    //========================================

    for (auto& entry : entries)
    {
        entry.infos = entry.effect->CreatePasses();

        bool valid = !entry.infos.empty() && entry.infos.back().target == nullptr;

        for (size_t p = 0; valid && p + 1 < entry.infos.size(); p++)
        {
            //最終パス以外は中間RenderTargetを持っている必要がある
            valid = (entry.infos[p].target != nullptr);
        }

        if (!valid)
        {
            OutputDebugStringW(L"PostEffectPipeline : CreatePasses の内容が不正(最後のパスだけ target = nullptr にする)\n");
            return false;
        }
    }

    //========================================
    // RenderPass生成
    // 描画先と有効/無効は ApplyRouting が決める
    //========================================

    scenePass = std::make_shared<RenderPass>();

    scenePass->SetExecuteFunction(sceneFunction);

    for (auto& entry : entries)
    {
        entry.passes.clear();

        for (auto& info : entry.infos)
        {
            auto pass = std::make_shared<RenderPass>();

            pass->SetExecuteFunction(info.execute);

            entry.passes.push_back(pass);
        }
    }

    //========================================
    // 描画先・入力テクスチャの決定
    //========================================

    ApplyRouting();

    //========================================
    // RenderPass登録(シーン -> エフェクト追加順)
    //========================================

    auto passManager = RenderPassManager::GetInstance();

    passManager->AddPass(scenePass);

    for (auto& entry : entries)
    {
        for (auto& pass : entry.passes)
        {
            passManager->AddPass(pass);
        }
    }

    isBuilt = true;

    return true;
}

void PostEffectPipeline::ApplyRouting()
{
    //有効なエフェクトの数
    size_t activeCount = 0;

    for (auto& entry : entries)
    {
        if (entry.effect->IsEnabled())
        {
            activeCount++;
        }
    }

    //有効なエフェクトがあればRenderTargetへ描画、無ければBackBufferへ直接描画
    scenePass->SetRenderTarget(activeCount == 0 ? nullptr : sceneRenderTarget.get());

    //次のエフェクトの入力になる RenderTarget (最初はシーンの描画結果)
    RenderTarget* input = sceneRenderTarget.get();

    //有効なエフェクトの中での順番
    size_t activeIndex = 0;

    for (auto& entry : entries)
    {
        const bool enabled = entry.effect->IsEnabled();

        //無効なエフェクトのパスは実行しない(入力はそのまま次へ渡る)
        for (auto& pass : entry.passes)
        {
            pass->SetEnabled(enabled);
        }

        if (!enabled)
        {
            continue;
        }

        //このエフェクトの出力先
        //  最後 : BackBuffer(nullptr)
        //  途中 : 中間RenderTargetを交互に使う(入力と出力が同じにならない)
        const bool isLast = (activeIndex + 1 == activeCount);

        RenderTarget* output = isLast ? nullptr : workRenderTargets[activeIndex % 2].get();

        //入力テクスチャ設定
        entry.effect->SetInputTexture(input->GetSRV());

        //target が nullptr のパスが最終出力 -> ここで決めた出力先を使う
        for (size_t i = 0; i < entry.passes.size(); i++)
        {
            RenderTarget* target = entry.infos[i].target;

            entry.passes[i]->SetRenderTarget(target != nullptr ? target : output);
        }

        //このエフェクトの出力が次の入力になる
        input = output;

        activeIndex++;
    }
}
