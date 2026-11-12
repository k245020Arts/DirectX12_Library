#include "ShadowMap.h"
#include "../Engine/Engine.h"
#include "../../DirectX12_Library/d3dx12.h"
#include <cassert>

ShadowMap::~ShadowMap()
{
    ReleaseSRV();
}

void ShadowMap::ReleaseSRV()
{
    if (srvHandle == nullptr)
    {
        return;
    }

    DescriptorHeap::GetInstance()->Free(srvHandle->handleCPU, srvHandle->handleGPU);
    srvHandle = nullptr;
}

bool ShadowMap::Create(UINT _width, UINT _height)
{
    if (resource.Get() != nullptr || _width == 0 || _height == 0)
    {
        return false;
    }

    auto* device = Engine::GetInstance()->Device();
    if (device == nullptr)
    {
        return false;
    }

    width = _width;
    height = _height;

    // DSVではD32_FLOAT、SRVではR32_FLOATとして同じ深度リソースを参照する。
    const auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS,width,height,1,1,1,0,D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    HRESULT hr = device->CreateCommittedResource(&heapProperties,D3D12_HEAP_FLAG_NONE,&resourceDesc,D3D12_RESOURCE_STATE_DEPTH_WRITE,&clearValue,IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
    );
    
    if (FAILED(hr))
    {
        resource.Reset();
        return false;
    }

    currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    // ShadowMap専用のDSV Heap（Shader-visibleにする必要はない）。
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf()));

    if (FAILED(hr))
    {
        resource.Reset();
        dsvHeap.Reset();
        return false;
    }

    dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(resource.Get(), &dsvDesc, dsvHandle);

    // SRVは共通のShader-visible DescriptorHeapから確保する。
    srvHandle = DescriptorHeap::GetInstance()->Allocate();
    if (srvHandle == nullptr)
    {
        resource.Reset();
        dsvHeap.Reset();
        dsvHandle = {};
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    device->CreateShaderResourceView(resource.Get(), &srvDesc, srvHandle->handleCPU);

    return true;
}

void ShadowMap::Transition(D3D12_RESOURCE_STATES nextState)
{
    if (resource.Get() == nullptr || currentState == nextState)
    {
        return;
    }

    auto* commandList = Engine::GetInstance()->CommandList();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),currentState,nextState);

    commandList->ResourceBarrier(1, &barrier);
    currentState = nextState;
}

void ShadowMap::SetSize(int _w, int _h)
{
    width = _w;
    height = _h;

    resource.Reset();
    dsvHeap.Reset();
    dsvHandle = {};
    srvHandle = nullptr;
    Create(_w, _h);
}

void ShadowMap::BeginRender(float clearDepth)
{
    if (!IsValid())
    {
        return;
    }

    Transition(D3D12_RESOURCE_STATE_DEPTH_WRITE);

    auto* commandList = Engine::GetInstance()->CommandList();

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor{};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(width);
    scissor.bottom = static_cast<LONG>(height);

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    // 深度専用パスなのでRTVはバインドしない。
    commandList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
    commandList->ClearDepthStencilView(dsvHandle,D3D12_CLEAR_FLAG_DEPTH,clearDepth,0,0,nullptr);
}

void ShadowMap::EndRender()
{
    if (!IsValid())
    {
        return;
    }

    Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Engine::GetInstance()->ResetViewport();
}
