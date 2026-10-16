#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "../Comptr.h"
#include "../SingleTon/SingletonBase.h"

class ConstantBuffer;
class TextureLoader;

struct DescriptorHandle
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
}; 

class DescriptorHeap : public SingletonBase<DescriptorHeap>
{
public:
	DescriptorHeap(); // コンストラクタで生成する
	ComPtr<ID3D12DescriptorHeap> GetHeap(); // ディスクリプタヒープを返す
	DescriptorHandle* Register(TextureLoader* texture, const std::string& _path); // テクスチャーをディスクリプタヒープに登録し、ハンドルを返す
	DescriptorHandle* Register(TextureLoader* texture, const std::wstring& _path); // テクスチャーをディスクリプタヒープに登録し、ハンドルを返す

	DescriptorHandle* Allocate();

	void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu);

private:

	bool m_IsValid = false; // 生成に成功したかどうか
	UINT m_IncrementSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pHeap = nullptr; // ディスクリプタヒープ本体
	std::vector<DescriptorHandle*> m_pHandles; // 登録されているハンドル

	// 追加：空きスロットの管理
	std::vector<UINT> m_FreeIndices;
	std::vector<bool> m_Used;

	friend class SingletonBase<DescriptorHeap>;
};