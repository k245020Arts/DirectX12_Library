#include "TextureLoader.h"
#include <DirectXTex.h>
#include <filesystem>
#include <algorithm>
#include "../Engine/Engine.h"

// 拡張子を返す
std::wstring FileExtension(const std::wstring& path)
{
	auto idx = path.rfind(L'.');

	assert(idx != std::wstring::npos);

	auto result = path.substr(idx + 1);
	return result;
}

TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader()
{

}

bool TextureLoader::Load(const std::string& _path)
{
    return Load(GetWideString(_path));
}

bool TextureLoader::Load(const std::wstring& _path)
{
	//既にロード済みか確認
	auto it = textureBuffer.find(_path);

	if (it != textureBuffer.end())
	{
		return false;
	}

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};

	auto ext = FileExtension(_path);

	HRESULT hr = S_FALSE;

	std::string pngString = "png";
	std::string tgaString = "tga";

	auto png = GetWideString(pngString);
	auto tga = GetWideString(tgaString);

	if (ext == png) // pngの時はWICFileを使う
	{
		hr = DirectX::LoadFromWICFile(_path.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImg);
	}
	else if (ext == tga) // tgaの時はTGAFileを使う
	{
		hr = DirectX::LoadFromTGAFile(_path.c_str(), &metadata, scratchImg);
	}

	if (FAILED(hr))
	{
		ColorTexture(_path, Vector4(1.0f, 0.0f, 1.0f, 1.0f));
		return false;
	}

	CreateTexture(metadata,scratchImg,_path);
    return true;
}

TextureData* TextureLoader::Get(const std::wstring& path)
{
	auto it = textureBuffer.find(path);

	if (it == textureBuffer.end())
	{
		return nullptr;
	}

	return &it->second;
}

TextureData* TextureLoader::Get(const std::string& path)
{
	return Get(GetWideString(path));
}

bool TextureLoader::ColorTexture(const std::wstring& _name,const DirectX::XMFLOAT4& _color)
{
	// すでに作られているなら作らない
	auto it = textureBuffer.find(_name);

	if (it != textureBuffer.end())
	{
		return true;
	}

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};

	// 1×1 RGBAテクスチャ
	HRESULT result = scratchImg.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM,1,1,1,1);

	if (FAILED(result))
	{
		return false;
	}

	//Metadata取得
	metadata = scratchImg.GetMetadata();

	// 画像データ取得
	const DirectX::Image* image = scratchImg.GetImage(0, 0, 0);

	if (image == nullptr)
	{
		return false;
	}

	// RGBAを設定
	image->pixels[0] = static_cast<uint8_t>(_color.x * 255.0f);

	image->pixels[1] = static_cast<uint8_t>(_color.y * 255.0f);
											
	image->pixels[2] = static_cast<uint8_t>(_color.z * 255.0f);
											
	image->pixels[3] = static_cast<uint8_t>(_color.w * 255.0f);
	 
	// 通常のテクスチャと同じ処理へ
	return CreateTexture(metadata,scratchImg,_name);
}

D3D12_SHADER_RESOURCE_VIEW_DESC TextureLoader::ViewDesc(const std::string& _path)
{
	return ViewDesc(GetWideString(_path));
}

D3D12_SHADER_RESOURCE_VIEW_DESC TextureLoader::ViewDesc(const std::wstring& _path)
{
	//シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = textureBuffer[_path].format; //0.0f～1.0fに初期化 DXGI_FORMAT_R8G8B8A8_UNORM
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクステャ用

	srvDesc.Texture2D.MipLevels = 1; //ミップマップを使用しないので1

	return srvDesc;
}

bool TextureLoader::CreateTexture(DirectX::TexMetadata& metadata, DirectX::ScratchImage& scratchImg, const std::wstring& _path)
{
	auto image = scratchImg.GetImage(0, 0, 0);

	//テクステャバッファーの作成

	D3D12_HEAP_PROPERTIES upLoadheapProp = {};

	//UpLoadにする
	upLoadheapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	//アップロード用に使用する前提なのでUNKNOWNで良い
	upLoadheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upLoadheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//単一アダプタのため0
	upLoadheapProp.CreationNodeMask = 0;
	upLoadheapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};

	resDesc.Format = DXGI_FORMAT_UNKNOWN; //単なるデータの固まりなのでUNKONWN
	resDesc.Width = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * image->height; //データサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1; //通常テクステャなのでアンチエイリシアリングしない
	resDesc.SampleDesc.Quality = 0; //最低クオリティ
	resDesc.MipLevels = 1; //ミップアップしないのでミップ数は1つ : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //単なるバッファとして生成
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //連続したレイアウト
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //フラグなし

	ComPtr<ID3D12Resource> upLoadBuffer = nullptr;

	auto result = Engine::GetInstance()->Device()->CreateCommittedResource(&upLoadheapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upLoadBuffer));

	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT; //テクスチャ用

	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//単一アダプタのため0
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	resDesc.Format = metadata.format; //RGBAフォーマットDXGI_FORMAT_R8G8B8A8_UNORM : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Width = static_cast<UINT>(metadata.width); //幅 255 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Height = (UINT)metadata.height; //高さ 255 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.DepthOrArraySize = (UINT16)metadata.arraySize; //2Dで配列でもないので1 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.SampleDesc.Count = 1; //通常テクステャなのでアンチエイリシアリングしない
	resDesc.SampleDesc.Quality = 0; //最低クオリティ
	resDesc.MipLevels = (UINT16)metadata.mipLevels; //ミップアップしないのでミップ数は1つ : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension); //２Dテクステャ用 D3D12_RESOURCE_DIMENSION_TEXTURE2D : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; //レイアウトは設定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //フラグなし

	auto size = textureBuffer.size();

	ComPtr<ID3D12Resource> texBuff = nullptr;

	result = Engine::GetInstance()->Device()->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texBuff));
	//GPUにデータ転送
	//result = texbuff->WriteToSubresource(0, nullptr, texturedata.data(), sizeof(TexRGBA) * 256, sizeof(TexRGBA) * texturedata.size());
	//result = texbuff->WriteToSubresource(0, nullptr, image->pixels, (UINT)image->rowPitch, (UINT)image->slicePitch);

	uint8_t* mapforImg = nullptr;//image->pixelsと同じ型にする
	result = upLoadBuffer->Map(0, nullptr, (void**)&mapforImg);//マップ
	auto srcAddress = image->pixels;
	auto rowPitch = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < image->height; ++y) {

		std::copy_n(srcAddress, image->rowPitch, mapforImg);//コピー
		//1行ごとの辻褄を合わせてやる
		srcAddress += image->rowPitch;
		mapforImg += rowPitch;
	}

	D3D12_TEXTURE_COPY_LOCATION src = {};

	//コピー元(アップロード側)の設定
	src.pResource = upLoadBuffer.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; //フットプリント指定
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = (UINT)metadata.width;
	src.PlacedFootprint.Footprint.Height = (UINT)metadata.height;
	src.PlacedFootprint.Footprint.Depth = (UINT)metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = image->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	//コピー先指定
	dst.pResource = texBuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; //インデックスの指定
	dst.SubresourceIndex = 0;

	//Engine::GetInstance()->CommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	Engine::GetInstance()->UploadTexture(upLoadBuffer.Get(), texBuff.Get(),src,dst);

	TextureData data;

	data.texture = texBuff;
	data.upload = upLoadBuffer;
	data.width = static_cast<UINT>(metadata.width);
	data.height = static_cast<UINT>(metadata.height);
	data.format = metadata.format;

	textureBuffer.emplace(_path, data);

    return true;
}