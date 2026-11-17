#include "MultiRenderTarget.h"
#include "../Engine/Engine.h"

bool MultiRenderTarget::Create(UINT _width, UINT _height, const DXGI_FORMAT* _formats, const Vector4* _clearColors, UINT _count)
{
    // D3D12 が同時に使えるターゲットは最大8枚
    if (_formats == nullptr || _clearColors == nullptr || _count == 0 || _count > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
    {
        return false;
    }

    targets.clear();

    for (UINT i = 0; i < _count; i++)
    {
        auto target = std::make_unique<RenderTarget>();

        if (!target->Create(_width, _height, _formats[i], _clearColors[i]))
        {
            targets.clear();
            return false;
        }

        targets.push_back(std::move(target));
    }

    return true;
}

void MultiRenderTarget::BeginRender()
{
    if (targets.empty())
    {
        return;
    }

    auto engine = Engine::GetInstance();

    auto commandList = engine->CommandList();

    // 全ターゲットを描画可能な状態にして、RTVを並べる
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;

    rtvHandles.reserve(targets.size());

    for (auto& target : targets)
    {
        target->ToRenderTargetState();

        rtvHandles.push_back(target->GetRTVHandle());
    }

    // ビューポート・シザーは全ターゲット共通(同じ大きさ)
    const UINT width = targets[0]->GetWidth();
    const UINT height = targets[0]->GetHeight();

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(width);
    scissor.bottom = static_cast<LONG>(height);

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    // RTVはバラバラの場所にあるので、第3引数は FALSE(配列で渡す)
    auto dsvHandle = engine->GetDSVHandle();

    commandList->OMSetRenderTargets(static_cast<UINT>(rtvHandles.size()), rtvHandles.data(), FALSE, &dsvHandle);

    // 深度は1回だけクリア、色は1枚ずつ自分のクリア色でクリア
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    for (size_t i = 0; i < targets.size(); i++)
    {
        const Vector4 color = targets[i]->GetClearColor();

        const float clearColor[4] = { color.x, color.y, color.z, color.w };

        commandList->ClearRenderTargetView(rtvHandles[i], clearColor, 0, nullptr);
    }
}

void MultiRenderTarget::EndRender()
{
    for (auto& target : targets)
    {
        target->ToShaderResourceState();
    }

    Engine::GetInstance()->ResetViewport();
}
