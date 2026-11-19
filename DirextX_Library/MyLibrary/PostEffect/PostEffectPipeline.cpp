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

    //シーン描画用のマルチレンダーターゲット(色 + Bloom など。構成は SceneLayout.h)
    sceneTargets = std::make_shared<MultiRenderTarget>();

    if (!sceneTargets->Create(width, height, SceneLayout::Formats, SceneLayout::ClearColors, SceneLayout::Count))
    {
        sceneTargets.reset();
        return false;
    }

    //エフェクトへ渡すテクスチャ一覧
    sceneTextures.color = sceneTargets->GetTarget(SceneLayout::Color)->GetSRV();
    sceneTextures.bloom = sceneTargets->GetTarget(SceneLayout::Bloom)->GetSRV();

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

    //シーンは常にマルチレンダーターゲットへ描く(全ターゲットのバインド・クリア・状態遷移はここで行う)
    scenePass->SetBeginFunction([this]() { sceneTargets->BeginRender(); });
    scenePass->SetEndFunction([this]()
        {
            sceneTargets->EndRender();

            //深度が必要なエフェクトがあるときは、このあとのパスで深度バッファが消されるので、ここでコピーしておく
            if (needSceneDepth)
            {
                Engine::GetInstance()->CopyDepthBuffer();
            }
        });

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
    // コピー用のエフェクト
    // 有効なエフェクトが無いときに、シーンの色をBackBufferへ出す
    //========================================

    copyEffect = std::make_unique<CopyEffect>();

    if (!copyEffect->Init())
    {
        OutputDebugStringW(L"PostEffectPipeline : CopyEffectの初期化に失敗\n");
        return false;
    }

    copyPass = std::make_shared<RenderPass>();

    copyPass->SetExecuteFunction([this]() { copyEffect->Draw(); });

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

    //最後にコピー(有効なエフェクトが無いときだけ実行される)
    passManager->AddPass(copyPass);

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

    //深度が必要なエフェクト(フォグなど)が有効なら、シーン描画後に深度をコピーして、SceneTextures::depth で渡す
    needSceneDepth = false;

    for (auto& entry : entries)
    {
        if (entry.effect->IsEnabled() && entry.effect->NeedsSceneDepth())
        {
            needSceneDepth = true;
        }
    }

    sceneTextures.depth = needSceneDepth ? Engine::GetInstance()->GetDepthSRV() : nullptr;

    //シーンは常にマルチレンダーターゲットへ描く(Beginで全ターゲットをバインドするので、ここでは描画先を指定しない)

    //次のエフェクトの入力になる RenderTarget (最初はシーンの色)
    RenderTarget* input = sceneTargets->GetTarget(SceneLayout::Color);

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

    //有効なエフェクトが無いときは、シーンの色をBackBufferへコピーする(それ以外のときは実行しない)
    copyPass->SetEnabled(activeCount == 0);

    if (activeCount == 0)
    {
        copyEffect->SetInputTexture(sceneTextures.color);

        copyPass->SetRenderTarget(nullptr);
    }
}
