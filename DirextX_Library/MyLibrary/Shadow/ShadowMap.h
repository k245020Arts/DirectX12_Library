#pragma once

#include "../Comptr.h"
#include "../DescriptorHeap/DescriptorHeap.h"

class ShadowMap
{
public:

    ShadowMap() = default;
    ~ShadowMap();

    bool Create(UINT width, UINT height);

    void BeginRender();
    void EndRender();

    DescriptorHandle* GetSRV() const
    {
        return srvHandle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const
    {
        return dsvHandle;
    }

    ID3D12Resource* GetResource() const
    {
        return resource.Get();
    }

    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }

private:

    void Transition(D3D12_RESOURCE_STATES nextState);

    ComPtr<ID3D12Resource> resource;

    ComPtr<ID3D12DescriptorHeap> dsvHeap;

    DescriptorHandle* srvHandle = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};

    UINT width = 0;
    UINT height = 0;

    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
};