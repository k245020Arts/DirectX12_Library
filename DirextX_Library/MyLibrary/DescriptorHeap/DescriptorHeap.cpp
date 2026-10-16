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
	// 空きスロットがあれば再利用
	if (!m_FreeIndices.empty())
	{
		UINT index = m_FreeIndices.back();
		m_FreeIndices.pop_back();

		m_Used[index] = true;
		return m_pHandles[index];
	}

	// 新規スロットの確保
	const size_t index = m_pHandles.size();

	if (index >= HANDLE_MAX)
	{
		return nullptr;
	}

	auto* handle = new DescriptorHandle();

	handle->handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();

	handle->handleCPU.ptr += m_IncrementSize * index;

	handle->handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();

	handle->handleGPU.ptr += m_IncrementSize * index;

	m_pHandles.push_back(handle);
	m_Used.push_back(true);

	return handle;
}

void DescriptorHeap::Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu,D3D12_GPU_DESCRIPTOR_HANDLE gpu)
{
	const auto cpuStart = m_pHeap->GetCPUDescriptorHandleForHeapStart();

	const auto gpuStart = m_pHeap->GetGPUDescriptorHandleForHeapStart();

	if (cpu.ptr < cpuStart.ptr ||
		gpu.ptr < gpuStart.ptr)
	{
		return;
	}

	const SIZE_T cpuOffset = cpu.ptr - cpuStart.ptr;
	const SIZE_T gpuOffset = gpu.ptr - gpuStart.ptr;

	// スロットの境界と CPU/GPU ハンドルの対応を確認
	if (cpuOffset % m_IncrementSize != 0 ||
		gpuOffset % m_IncrementSize != 0 ||
		cpuOffset != gpuOffset)
	{
		return;
	}

	const UINT index = static_cast<UINT>(cpuOffset / m_IncrementSize);

	if (index >= m_Used.size() || !m_Used[index])
	{
		return;
	}

	m_Used[index] = false;
	m_FreeIndices.push_back(index);
}