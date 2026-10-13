#include "IndexBuffer.h"
#include "../Engine/Engine.h"

IndexBuffer::IndexBuffer(size_t size, const uint32_t* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // ヒーププロパティ
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// リソースの設定

	// リソースを生成
	auto hr = Engine::GetInstance()->Device()->CreateCommittedResource(&prop,D3D12_HEAP_FLAG_NONE,&desc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(pBuffer.GetAddressOf()));

	if (FAILED(hr))
	{
		printf("[OnInit] インデックスバッファリソースの生成に失敗");
		return;
	}

	// インデックスバッファビューの設定
	view = {};
	view.BufferLocation = pBuffer->GetGPUVirtualAddress();
	view.Format = DXGI_FORMAT_R32_UINT;
	view.SizeInBytes = static_cast<UINT>(size);

	BufferMapping(pInitData);
	success = true;
}

bool IndexBuffer::IsSuccess()
{
	return success;
}

const D3D12_INDEX_BUFFER_VIEW& IndexBuffer::View() const
{
	return view;
}

void IndexBuffer::BufferMapping(const void* pInitData)
{

	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		auto hr = pBuffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("[OnInit] インデックスバッファマッピングに失敗");
			return;
		}

		// インデックスデータをマッピング先に設定
		memcpy(ptr, pInitData, view.SizeInBytes);

		// マッピング解除
		pBuffer->Unmap(0, nullptr);
	}
}

void IndexBuffer::Resize(size_t _size)
{
	if (_size == 0)
	{
		return;
	}

	// 新しいサイズを保存
	bufferSize = _size;

	// 古いリソースを破棄
	pBuffer.Reset();

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // ヒーププロパティ
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(_size);	// リソースの設定

	// リソースを生成
	auto hr = Engine::GetInstance()->Device()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pBuffer.GetAddressOf()));

	if (FAILED(hr))
	{
		printf("[OnInit] インデックスバッファリソースの生成に失敗");
		return;
	}

	// インデックスバッファビューの設定
	view = {};
	view.BufferLocation = pBuffer->GetGPUVirtualAddress();
	view.Format = DXGI_FORMAT_R32_UINT;
	view.SizeInBytes = static_cast<UINT>(_size);
}