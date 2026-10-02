#pragma once
#include <d3d12.h>
#include "../Comptr.h"

class VertexBuffer
{
public:
	VertexBuffer(size_t size, size_t stride, const void* pInitData); // コンストラクタでバッファを生成
	const D3D12_VERTEX_BUFFER_VIEW& GetView() const; // 頂点バッファビューを取得
	bool IsSuccess(); // バッファの生成に成功したかを取得
	void BufferMapping(const void* pInitData);
	void Resize(size_t _size);

private:
	bool success = false; // バッファの生成に成功したかを取得
	ComPtr<ID3D12Resource> pBuffer = nullptr; // バッファ
	D3D12_VERTEX_BUFFER_VIEW view = {}; // 頂点バッファビュー

	VertexBuffer(const VertexBuffer&) = delete;
	void operator = (const VertexBuffer&) = delete;

	size_t bufferSize;
	size_t strideSize;
};