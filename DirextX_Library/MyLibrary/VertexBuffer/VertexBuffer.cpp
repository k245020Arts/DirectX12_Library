#include "VertexBuffer.h"
#include "../../DirectX12_Library/d3dx12.h"
#include "../Engine/Engine.h"

VertexBuffer::VertexBuffer(size_t size, size_t stride, const void* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); 	// ヒーププロパティ
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size); 	// リソースの設定

	bufferSize = size;
	strideSize = stride;
	// リソースを生成
	auto hr = Engine::GetInstance()->Device()->CreateCommittedResource(&prop,D3D12_HEAP_FLAG_NONE,&desc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(pBuffer.GetAddressOf()));

	if (FAILED(hr))
	{
		OutputDebugStringW(L"頂点バッファリソースの生成に失敗");
		return;
	}

	// 頂点バッファビューの設定
	view.BufferLocation = pBuffer->GetGPUVirtualAddress();
	view.SizeInBytes = static_cast<UINT>(size);
	view.StrideInBytes = static_cast<UINT>(stride);

	BufferMapping(pInitData);

	success = true;
}

const D3D12_VERTEX_BUFFER_VIEW& VertexBuffer::GetView() const
{
	return view;
}

bool VertexBuffer::IsSuccess()
{
	return success;
}

void VertexBuffer::BufferMapping(const void* pInitData)
{
	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		auto hr = pBuffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			OutputDebugStringW(L"頂点バッファマッピングに失敗");
			return;
		}

		// 頂点データをマッピング先に設定
		memcpy(ptr, pInitData, bufferSize);

		// マッピング解除
		pBuffer->Unmap(0, nullptr);
	}
}

void VertexBuffer::Resize(size_t _size)
{
	if (_size == 0)
	{
		return;
	}

	// 新しいサイズを保存
	bufferSize = _size;

	// 古いリソースを破棄
	pBuffer.Reset();

	auto device = Engine::GetInstance()->Device();

	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_size);

	auto hr = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pBuffer)
	);

	if (FAILED(hr))
	{
		OutputDebugStringW(L"頂点バッファのResizeに失敗\n");

		bufferSize = 0;
		return;
	}

	// Viewを更新
	view.BufferLocation = pBuffer->GetGPUVirtualAddress();
	view.SizeInBytes = static_cast<UINT>(_size);
	view.StrideInBytes = static_cast<UINT>(strideSize);
}