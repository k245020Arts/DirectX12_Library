#include "ShadowMap.h"
#include "../Engine/Engine.h"
#include "../../DirectX12_Library/d3dx12.h"

ShadowMap::~ShadowMap()
{
    if (srvHandle != nullptr)
    {
        DescriptorHeap::GetInstance()->Free(srvHandle->handleCPU,srvHandle->handleGPU);

        srvHandle = nullptr;
    }
}


bool ShadowMap::Create(UINT _width, UINT _height)
{
    auto device = Engine::GetInstance()->Device();

    if (device == nullptr || _width == 0 || _height == 0)
    {
        return false;
    }

    width = _width;
    height = _height;

    // -----------------------------
    // DSV Heap
    // -----------------------------

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

    heapDesc.Type =D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    heapDesc.NumDescriptors = 1;

    heapDesc.Flags =D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(&heapDesc,IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

    // -----------------------------
    // Depth Resource
    // -----------------------------

    auto heapProperties =CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    auto resourceDesc =CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS,width,height,1,1,1,0,D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    // 深度バッファの初期値
    D3D12_CLEAR_VALUE clearValue{};

    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    currentState =D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    hr = device->CreateCommittedResource(&heapProperties,D3D12_HEAP_FLAG_NONE,&resourceDesc,currentState,&clearValue,IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    // -----------------------------
    // DSV
    // -----------------------------

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;

    dsvDesc.ViewDimension =D3D12_DSV_DIMENSION_TEXTURE2D;

    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(resource.Get(),&dsvDesc,dsvHandle);

    // -----------------------------
    // SRV
    // -----------------------------

    srvHandle =DescriptorHeap::GetInstance()->Allocate();

    if (srvHandle == nullptr)
    {
        resource.Reset();
        dsvHeap.Reset();
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

    srvDesc.Shader4ComponentMapping =D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(resource.Get(),&srvDesc,srvHandle->handleCPU);

    return true;
}


void ShadowMap::Transition(D3D12_RESOURCE_STATES nextState)
{
    if (currentState == nextState)
    {
        return;
    }

    auto commandList =Engine::GetInstance()->CommandList();

    auto barrier =CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),currentState,nextState);

    commandList->ResourceBarrier(1, &barrier);

    currentState = nextState;
}

void ShadowMap::BeginRender()
{
    // 深度書き込み可能な状態にする
    Transition(D3D12_RESOURCE_STATE_DEPTH_WRITE);

    auto commandList = Engine::GetInstance()->CommandList();

    // ShadowMap専用Viewport
    D3D12_VIEWPORT viewport{};

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    commandList->RSSetViewports(1, &viewport);

    // ShadowMap専用Scissor
    D3D12_RECT scissor{};

    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(width);
    scissor.bottom = static_cast<LONG>(height);

    commandList->RSSetScissorRects(1, &scissor);

    // 深度専用の描画先を設定
    commandList->OMSetRenderTargets(0,nullptr,FALSE,&dsvHandle);

    // 深度をクリア
    commandList->ClearDepthStencilView(dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr);
}

void ShadowMap::EndRender()
{
    Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}