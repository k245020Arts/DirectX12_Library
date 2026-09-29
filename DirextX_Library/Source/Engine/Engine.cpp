#include "Engine.h"

bool Engine::Init(HWND _hwnd, const Size& _size)
{
    hwnd = _hwnd;

    windowSize = _size;

    bool deviceCreate = CreateDevice();

    if (!deviceCreate) {
        OutputDebugStringW(L"デバイスの出力に失敗しました");
        return false;
    }



    return true;
}

void Engine::BeginRender()
{
}

void Engine::EndRender()
{
}

ID3D12Device6* Engine::Device()
{
    return nullptr;
}

ID3D12GraphicsCommandList* Engine::CommandList()
{
    return nullptr;
}

UINT Engine::CurrentBackBufferIndex()
{
    return 0;
}

bool Engine::CreateDevice()
{

    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice));

    return SUCCEEDED(hr);
}

bool Engine::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    HRESULT hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pQueue));

    return false;
}

bool Engine::CreateSwapChain()
{
    return false;
}

bool Engine::CreateCommandList()
{
    return false;
}

bool Engine::CreateFence()
{
    return false;
}

void Engine::CreateViewPort()
{
}

void Engine::CreateScissorRect()
{
}

bool Engine::CreateRenderTarget()
{
    return false;
}

bool Engine::CreateDepthStencil()
{
    return false;
}

void Engine::WaitRender()
{
}
