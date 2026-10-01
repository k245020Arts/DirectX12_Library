#pragma once
#include <cstdint>
#include "../../DirectX12_Library/d3dx12.h"
#include "../ComPtr.h"

class IndexBuffer
{
public:
	IndexBuffer(size_t size, const uint32_t* pInitData = nullptr);
	bool IsSuccess();
	const D3D12_INDEX_BUFFER_VIEW& View() const;

	void BufferMapping(size_t size, const void* pInitData);

private:
	bool success = false;
	ComPtr<ID3D12Resource> pBuffer; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW view; // インデックスバッファビュー

	IndexBuffer(const IndexBuffer&) = delete;
	void operator = (const IndexBuffer&) = delete;
};

