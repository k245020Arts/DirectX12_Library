#pragma once
#include "../../DirectX12_Library/d3dx12.h"
#include "../Comptr.h"

class ConstBuffer
{
public:
    ConstBuffer(size_t size); // コンストラクタで定数バッファを生成
    bool IsValid(); // バッファ生成に成功したかを返す
    D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const; // バッファのGPU上のアドレスを返す
    const D3D12_CONSTANT_BUFFER_VIEW_DESC& ViewDesc()const; // 定数バッファビューを返す

    void* GetPtr() const; // 定数バッファにマッピングされたポインタを返す

    template<typename T>
    T* GetPtr()
    {
        return (T*)(GetPtr());
    }

private:
    bool isSuccess = false; // 定数バッファ生成に成功したか
    ComPtr<ID3D12Resource> m_pBuffer; // 定数バッファ
    D3D12_CONSTANT_BUFFER_VIEW_DESC m_Desc; // 定数バッファビューの設定
    void* m_pMappedPtr = nullptr;

    ConstBuffer(const ConstBuffer&) = delete;
    void operator = (const ConstBuffer&) = delete;
};

