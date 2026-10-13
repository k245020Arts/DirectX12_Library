#include "DescriptorHeap.h"
#include "../Texture/TextureLoader.h"
#include "../Engine/Engine.h"

const UINT HANDLE_MAX = 512;

DescriptorHeap::DescriptorHeap()
{
	m_pHandles.clear();
	m_pHandles.reserve(HANDLE_MAX);

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = HANDLE_MAX;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto device = Engine::GetInstance()->Device();

	// ディスクリプタヒープを生成
	auto hr = device->CreateDescriptorHeap(&desc,IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf()));

	if (FAILED(hr))
	{
		m_IsValid = false;
		return;
	}

	m_IncrementSize = device->GetDescriptorHandleIncrementSize(desc.Type); // ディスクリプタヒープ1個のメモリサイズを返す
	m_IsValid = true;
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeap::GetHeap()
{
	return m_pHeap;
}

DescriptorHandle* DescriptorHeap::Register(TextureLoader* texture,const std::string& _path)
{
	auto count = m_pHandles.size();
	if (HANDLE_MAX <= count)
	{
		return nullptr;
	}

	DescriptorHandle* pHandle = new DescriptorHandle();

	auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleCPU.ptr += m_IncrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleGPU.ptr += m_IncrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	pHandle->handleCPU = handleCPU;
	pHandle->handleGPU = handleGPU;

	auto device = Engine::GetInstance()->Device();
	auto textureData = texture->Get(_path);
	if (textureData == nullptr) {
		assert(false && "適切なパスがないです");
	}
	auto resource = textureData->texture.Get();
	auto desc = texture->ViewDesc(_path);
	device->CreateShaderResourceView(resource, &desc, pHandle->handleCPU); // シェーダーリソースビュー作成

	m_pHandles.push_back(pHandle);
	return pHandle; // ハンドルを返す
}

DescriptorHandle* DescriptorHeap::Register(TextureLoader* texture, const std::wstring& _path)
{
	auto count = m_pHandles.size();
	if (HANDLE_MAX <= count)
	{
		return nullptr;
	}

	DescriptorHandle* pHandle = new DescriptorHandle();

	auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleCPU.ptr += m_IncrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleGPU.ptr += m_IncrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	pHandle->handleCPU = handleCPU;
	pHandle->handleGPU = handleGPU;

	auto device = Engine::GetInstance()->Device();
	auto textureData = texture->Get(_path);
	if (textureData == nullptr) {
		assert(false && "適切なパスがないです");
	}
	auto resource = textureData->texture.Get();
	auto desc = texture->ViewDesc(_path);
	device->CreateShaderResourceView(resource, &desc, pHandle->handleCPU); // シェーダーリソースビュー作成

	m_pHandles.push_back(pHandle);
	return pHandle; // ハンドルを返す
}

DescriptorHandle* DescriptorHeap::Allocate()
{
	auto count = m_pHandles.size();

	if (HANDLE_MAX <= count)
	{
		return nullptr;
	}

	DescriptorHandle* pHandle = new DescriptorHandle();

	auto handleCPU =
		m_pHeap->GetCPUDescriptorHandleForHeapStart();

	handleCPU.ptr += m_IncrementSize * count;

	auto handleGPU =
		m_pHeap->GetGPUDescriptorHandleForHeapStart();

	handleGPU.ptr += m_IncrementSize * count;

	pHandle->handleCPU = handleCPU;
	pHandle->handleGPU = handleGPU;

	m_pHandles.push_back(pHandle);

	return pHandle;
}