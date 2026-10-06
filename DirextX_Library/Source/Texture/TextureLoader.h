#pragma once
#include "../ShaderStruct/ShaderStruct.h"
#include <unordered_map>
#include "../SingleTon/SingletonBase.h"
#include <DirectXTex.h>
#include "../Comptr.h"

struct TextureData
{
	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12Resource> upload;

	UINT width = 0;
	UINT height = 0;

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
};

class TextureLoader : public SingletonBase<TextureLoader>
{
public:
	TextureLoader();
	~TextureLoader();

	bool Load(const std::string& _path);
	bool Load(const std::wstring& _path);
	
	bool IsSuccess(); // 正常に読み込まれているかどうかを返す

	// 読み込んだテクスチャを取得
	TextureData* Get(const std::wstring& path);
	TextureData* Get(const std::string& path);

	// 指定した色の1×1テクスチャを作成
	bool ColorTexture(const std::wstring& _name, const DirectX::XMFLOAT4& _color);

	D3D12_SHADER_RESOURCE_VIEW_DESC  ViewDesc(const std::string& _path);
	D3D12_SHADER_RESOURCE_VIEW_DESC  ViewDesc(const std::wstring& _path);

private:

	bool success;
	std::unordered_map<std::wstring, TextureData> textureBuffer;

	bool CreateTexture(DirectX::TexMetadata& metadata, DirectX::ScratchImage& scratchImg, const std::wstring& _path);

	friend class SingletonBase<TextureLoader>;

};
