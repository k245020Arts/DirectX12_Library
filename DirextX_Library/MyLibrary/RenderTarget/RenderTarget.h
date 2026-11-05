#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "../DescriptorHeap/DescriptorHeap.h"

class RenderTarget
{
public:

    RenderTarget() = default;
    ~RenderTarget();

    bool Create(UINT width,UINT height,DXGI_FORMAT format);

    void BeginRender(const Vector4& clearColor);

    void EndRender();

    void SetShaderResource();

    ID3D12Resource* GetResource() const
    {
        return resource.Get();
    }

    DescriptorHandle* GetSRV() const
    {
        return srvHandle;
    }

    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }

private:

    void Transition(D3D12_RESOURCE_STATES nextState);

    ComPtr<ID3D12Resource> resource;

    ComPtr<ID3D12DescriptorHeap> rtvHeap;

    DescriptorHandle* srvHandle = nullptr;

    UINT rtvDescriptorSize = 0;

    UINT width = 0;
    UINT height = 0;

    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
};

