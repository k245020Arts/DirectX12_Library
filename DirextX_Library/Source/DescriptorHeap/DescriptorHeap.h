#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include "../Comptr.h"

class ConstantBuffer;
class TextureLoader;

struct DescriptorHandle
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
}; 

class DescriptorHeap
{
public:
	DescriptorHeap(); // コンストラクタで生成する
	ComPtr<ID3D12DescriptorHeap> GetHeap(); // ディスクリプタヒープを返す
	DescriptorHandle* Register(TextureLoader* texture, const std::string& _path); // テクスチャーをディスクリプタヒープに登録し、ハンドルを返す

private:
	bool m_IsValid = false; // 生成に成功したかどうか
	UINT m_IncrementSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pHeap = nullptr; // ディスクリプタヒープ本体
	std::vector<DescriptorHandle*> m_pHandles; // 登録されているハンドル
};