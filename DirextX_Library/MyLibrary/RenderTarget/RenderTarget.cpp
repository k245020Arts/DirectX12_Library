#include "RenderTarget.h"
#include "../Engine/Engine.h"
#include "../../DirectX12_Library/d3dx12.h"

RenderTarget::~RenderTarget()
{
    if (srvHandle)
    {
        DescriptorHeap::GetInstance()->Free(srvHandle->handleCPU,srvHandle->handleGPU);

        srvHandle = nullptr;
    }
}

bool RenderTarget::Create(UINT _width,UINT _height,DXGI_FORMAT _format)
{
    auto device = Engine::GetInstance()->Device();

    width = _width;
    height = _height;

    // RTV用Heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = 1;

    if (FAILED(device->CreateDescriptorHeap( &heapDesc,IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()))))
    {
        return false;
    }

    rtvDescriptorSize =device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // RenderTarget Resource
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    auto resourceDesc =CD3DX12_RESOURCE_DESC::Tex2D(_format,width,height,1,1,1,0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue{};

    clearValue.Format = _format;
    clearValue.Color[0] = 0.25f;
    clearValue.Color[1] = 0.25f;
    clearValue.Color[2] = 0.25f;
    clearValue.Color[3] = 1.0f;

    //クリアバリューに関する設定を入れる
    HRESULT hr = device->CreateCommittedResource(&heapProperties,D3D12_HEAP_FLAG_NONE,&resourceDesc,currentState,&clearValue,IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));

    if (FAILED(hr))
    {
        return false;
    }

    // RTV作成
    auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    device->CreateRenderTargetView(resource.Get(), nullptr,rtvHandle);

    // SRV作成
    srvHandle = DescriptorHeap::GetInstance()->Allocate();

    if (srvHandle == nullptr)
    {
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

    srvDesc.Format = _format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    srvDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(resource.Get(),&srvDesc,srvHandle->handleCPU);

    return true;
}

void RenderTarget::Transition(D3D12_RESOURCE_STATES nextState)
{
    if (currentState == nextState)
    {
        return;
    }

    auto commandList = Engine::GetInstance()->CommandList();

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),currentState, nextState );

    commandList->ResourceBarrier(1, &barrier);

    currentState = nextState;
}

void RenderTarget::BeginRender(const Vector4& clearColor)
{
    Transition(D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto commandList = Engine::GetInstance()->CommandList();

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

    auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    auto dsvHandle =Engine::GetInstance()->GetDSVHandle();

    commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle );

    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0, nullptr);

    const float color[] =
    {
        clearColor.x,
        clearColor.y,
        clearColor.z,
        clearColor.w
    };

    commandList->ClearRenderTargetView(rtvHandle,color,0,nullptr);
}

void RenderTarget::EndRender()
{
    Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Engine::GetInstance()->ResetViewport();
}

void RenderTarget::SetShaderResource()
{
    // この時点でPIXEL_SHADER_RESOURCEであること
    assert(currentState ==D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}