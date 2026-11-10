#pragma once

#include <d3d12.h>
#include "../Comptr.h"
#include "../DescriptorHeap/DescriptorHeap.h"

// 深度を書き込むDSVと、シェーダーから読むSRVを1つのリソースで管理する。
class ShadowMap
{
public:
    ShadowMap() = default;
    ~ShadowMap();

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    bool Create(UINT width = 2048, UINT height = 2048);
    bool IsValid() const { return resource.Get() != nullptr && srvHandle != nullptr && dsvHeap.Get() != nullptr; }

    // ShadowRenderPassの開始・終了から呼ぶ。
    void BeginRender(float clearDepth = 1.0f);
    void EndRender();

    ID3D12Resource* GetResource() const { return resource.Get(); }
    DescriptorHandle* GetSRV() const { return srvHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return dsvHandle; }

    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }
    D3D12_RESOURCE_STATES GetCurrentState() const { return currentState; }

private:
    void Transition(D3D12_RESOURCE_STATES nextState);
    void ReleaseSRV();

    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    DescriptorHandle* srvHandle = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};

    UINT width = 0;
    UINT height = 0;
    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
};
