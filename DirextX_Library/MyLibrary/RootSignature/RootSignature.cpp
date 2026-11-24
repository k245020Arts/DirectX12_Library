#include "RootSignature.h"
#include "../Engine/Engine.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

// ============================================================================
//  ルートシグネチャのキャッシュ
//   シリアライズした中身(バイト列)が同じなら、同じ ID3D12RootSignature を返す。
//   どのクラスが何個 RootSignature を作っても、種類が同じなら実体は1つになる。
//   (PSO のキャッシュは、ルートシグネチャのポインタが同じかどうかで同じ PSO かを判断している)
// ============================================================================
namespace
{
	std::mutex rootSignatureMutex;
	std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> rootSignatureCache; // シリアライズ結果 -> 実体
	std::unordered_set<const ID3D12RootSignature*> rootSignatureSet;                 // キャッシュにあるものの一覧
	
	HRESULT CreateRootSignatureCached(ID3DBlob * _blob, ComPtr<ID3D12RootSignature>&_out)
	{
		const std::string key(static_cast<const char*>(_blob->GetBufferPointer()), _blob->GetBufferSize());
		{
			std::lock_guard<std::mutex> lock(rootSignatureMutex);
			auto it = rootSignatureCache.find(key);
			if (it != rootSignatureCache.end())
			{
				_out = it->second;
				return S_OK;
			}
		}
		ComPtr<ID3D12RootSignature> rootSignature;
		const HRESULT hr = Engine::GetInstance()->Device()->CreateRootSignature(
			0, // GPUが複数ある場合のノードマスク（今回は1個しか無い想定なので0）
			_blob->GetBufferPointer(),
			_blob->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));

		if (FAILED(hr))
		{
			return hr;
		}
		
		std::lock_guard<std::mutex> lock(rootSignatureMutex);
		auto result = rootSignatureCache.emplace(key, rootSignature); // 同時に作られていたら、先に入った方を使う
		rootSignatureSet.insert(result.first->second.Get());
		_out = result.first->second;
		return S_OK;
	}
}

RootSignature::RootSignature()
{
	
}

bool RootSignature::IsSuccess()
{
	return success;
}

ID3D12RootSignature* RootSignature::Get()
{
	return pRootSignature.Get();
}

void RootSignature::SetTextureRootSampler()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[2] = {};
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // b0の定数バッファを設定、全てのシェーダーから見えるようにする

	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	// rootParam[1] : t0
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0; // t0
	descTblRange.RegisterSpace = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグニチャの設定（設定したいルートパラメーターとスタティックサンプラーを入れる）
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam); // ルートパラメーターの個数をいれる
	desc.NumStaticSamplers = 1; // サンプラーの個数をいれる
	desc.pParameters = rootParam; // ルートパラメーターのポインタをいれる
	desc.pStaticSamplers = &samplerDesc; // サンプラーのポインタを入れる
	desc.Flags = flag; // フラグを設定

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		return;
	}

	// ルートシグネチャ生成
	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}

void RootSignature::SetNormalRootSignature()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[1] = {};
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // b0の定数バッファを設定、全てのシェーダーから見えるようにする

	// スタティックサンプラーの設定
	auto sampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

	// ルートシグニチャの設定（設定したいルートパラメーターとスタティックサンプラーを入れる）
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam); // ルートパラメーターの個数をいれる
	desc.NumStaticSamplers = 1; // サンプラーの個数をいれる
	desc.pParameters = rootParam; // ルートパラメーターのポインタをいれる
	desc.pStaticSamplers = &sampler; // サンプラーのポインタを入れる
	desc.Flags = flag; // フラグを設定

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		return;
	}

	// ルートシグネチャ生成
	hr = Engine::GetInstance()->Device()->CreateRootSignature(
		0, // GPUが複数ある場合のノードマスク（今回は1個しか無い想定なので0）
		pBlob->GetBufferPointer(), // シリアライズしたデータのポインタ
		pBlob->GetBufferSize(), // シリアライズしたデータのサイズ
		IID_PPV_ARGS(pRootSignature.GetAddressOf())); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}


void RootSignature::SetModelRootSignature()
{
	// RootSignatureの設定
	D3D12_ROOT_SIGNATURE_FLAGS flag =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// RootParameter
	CD3DX12_ROOT_PARAMETER rootParam[11] = {};

	// RootParameter 0 : b0 Transform
	rootParam[0].InitAsConstantBufferView(0,0,D3D12_SHADER_VISIBILITY_ALL);

	// RootParameter 1 : t0 MaterialTexture
	D3D12_DESCRIPTOR_RANGE materialRange = {};

	materialRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	materialRange.NumDescriptors = 1;
	materialRange.BaseShaderRegister = 0;
	materialRange.RegisterSpace = 0;
	materialRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].InitAsDescriptorTable(1,&materialRange,D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 2 : b1 Light
	rootParam[2].InitAsConstantBufferView(1,0,D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 3 : b2 Bone
	rootParam[3].InitAsConstantBufferView(2,0,D3D12_SHADER_VISIBILITY_ALL);

	//RootParameter 4 : t1 ShadowMap
	D3D12_DESCRIPTOR_RANGE shadowMapRange = {};

	shadowMapRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	shadowMapRange.NumDescriptors = 1;
	shadowMapRange.BaseShaderRegister = 1;
	shadowMapRange.RegisterSpace = 0;
	shadowMapRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[4].InitAsDescriptorTable(1,&shadowMapRange,D3D12_SHADER_VISIBILITY_PIXEL);

	// RootParameter 5 : t2 NormalMap
	D3D12_DESCRIPTOR_RANGE normalRange = {};

	normalRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	normalRange.NumDescriptors = 1;
	normalRange.BaseShaderRegister = 2;
	normalRange.RegisterSpace = 0;
	normalRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[5].InitAsDescriptorTable(1, &normalRange, D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 3 : b3 Normal
	rootParam[6].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);
	//RootParameter 3 : b4 Specular
	rootParam[7].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL);
	//RootParameter 8 : b5 Bloom
	rootParam[8].InitAsConstantBufferView(5, 0, D3D12_SHADER_VISIBILITY_ALL);

	// RootParameter 5 : t2 NormalMap
	D3D12_DESCRIPTOR_RANGE heightRange = {};

	heightRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	heightRange.NumDescriptors = 1;
	heightRange.BaseShaderRegister = 3;
	heightRange.RegisterSpace = 0;
	heightRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[9].InitAsDescriptorTable(1, &heightRange, D3D12_SHADER_VISIBILITY_ALL);

	//RootParameter 9 : b6 Height
	rootParam[10].InitAsConstantBufferView(6, 0, D3D12_SHADER_VISIBILITY_ALL);

	//Static Sampler : s0
	D3D12_STATIC_SAMPLER_DESC samplerDescs[5] = {};

	// Sampler 0 : Material Texture (s0)
	samplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[0].MipLODBias = 0.0f;
	samplerDescs[0].MaxAnisotropy = 1;
	samplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[0].MinLOD = 0.0f;
	samplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[0].ShaderRegister = 0;
	samplerDescs[0].RegisterSpace = 0;
	samplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Sampler 1 : ShadowMap (s1)
	samplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	samplerDescs[1].MipLODBias = 0.0f;
	samplerDescs[1].MaxAnisotropy = 1;
	samplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	samplerDescs[1].MinLOD = 0.0f;
	samplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[1].ShaderRegister = 1;
	samplerDescs[1].RegisterSpace = 0;
	samplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDescs[2] = samplerDescs[0];

	samplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// シャドウマップの範囲外は「影なし(深度1.0)」として扱う
	samplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	samplerDescs[2].MaxAnisotropy = 1;
	samplerDescs[2].ShaderRegister = 2;

	// Sampler 0 : Material Texture (s0)
	samplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				 
	samplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				 
	samplerDescs[3].MipLODBias = 0.0f;
	samplerDescs[3].MaxAnisotropy = 1;
	samplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[3].MinLOD = 0.0f;
	samplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
				 
	samplerDescs[3].ShaderRegister = 3;
	samplerDescs[3].RegisterSpace = 0;
	samplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Sampler 4 : HeightMap Texture (40)
	samplerDescs[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[4].MipLODBias = 0.0f;
	samplerDescs[4].MaxAnisotropy = 1;
	samplerDescs[4].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[4].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[4].MinLOD = 0.0f;
	samplerDescs[4].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[4].ShaderRegister = 4;
	samplerDescs[4].RegisterSpace = 0;
	samplerDescs[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.NumParameters = _countof(rootParam);
	desc.pParameters = rootParam;

	desc.NumStaticSamplers = _countof(samplerDescs);
	desc.pStaticSamplers = samplerDescs;

	desc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc,D3D_ROOT_SIGNATURE_VERSION_1_0,pBlob.GetAddressOf(),pErrorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}

		OutputDebugStringW(L"RootSignatureのシリアライズに失敗");

		success = false;
		return;
	}

	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"RootSignatureの生成に失敗");

		success = false;
		return;
	}

	success = true;
}

void RootSignature::Set3DRootSignature()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[4] = {};
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // b0の定数バッファを設定、全てのシェーダーから見えるようにする

	// rootParam[1] : t0
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0; // t0
	descTblRange.RegisterSpace = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParam[2] : b1（Light）
	rootParam[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	//rootParam[3].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);

	//RootParameter 3 : t1 ShadowMap
	D3D12_DESCRIPTOR_RANGE shadowMapRange = {};

	shadowMapRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	shadowMapRange.NumDescriptors = 1;
	shadowMapRange.BaseShaderRegister = 1;
	shadowMapRange.RegisterSpace = 0;
	shadowMapRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[3].InitAsDescriptorTable(1, &shadowMapRange, D3D12_SHADER_VISIBILITY_PIXEL);

	//Static Sampler : s0
	D3D12_STATIC_SAMPLER_DESC samplerDescs[3] = {};

	// Sampler 0 : Material Texture (s0)
	samplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[0].MipLODBias = 0.0f;
	samplerDescs[0].MaxAnisotropy = 1;
	samplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[0].MinLOD = 0.0f;
	samplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[0].ShaderRegister = 0;
	samplerDescs[0].RegisterSpace = 0;
	samplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Sampler 1 : ShadowMap (s1)
	samplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	samplerDescs[1].MipLODBias = 0.0f;
	samplerDescs[1].MaxAnisotropy = 1;
	samplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	samplerDescs[1].MinLOD = 0.0f;
	samplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[1].ShaderRegister = 1;
	samplerDescs[1].RegisterSpace = 0;
	samplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDescs[2] = samplerDescs[0];

	samplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// シャドウマップの範囲外は「影なし(深度1.0)」として扱う
	samplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	samplerDescs[2].MaxAnisotropy = 1;
	samplerDescs[2].ShaderRegister = 2;


	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.NumParameters = _countof(rootParam);
	desc.pParameters = rootParam;

	desc.NumStaticSamplers = 3;
	desc.pStaticSamplers = samplerDescs;

	desc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		return;
	}

	// ルートシグネチャ生成
	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}

void RootSignature::SetPostEffectRootSignature()
{
	// ポストエフェクト専用RootSignatureを作る
	// t0 : 入力テクスチャ
	// s0 : サンプラー

	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[2] = {};

	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // b0の定数バッファを設定、ピクセルシェーダーから見える
	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	// rootParam[1] : t0
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0; // t0
	descTblRange.RegisterSpace = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグニチャの設定（設定したいルートパラメーターとスタティックサンプラーを入れる）
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam); // ルートパラメーターの個数をいれる
	desc.NumStaticSamplers = 1; // サンプラーの個数をいれる
	desc.pParameters = rootParam; // ルートパラメーターのポインタをいれる
	desc.pStaticSamplers = &samplerDesc; // サンプラーのポインタを入れる
	desc.Flags = flag; // フラグを設定

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		return;
	}

	// ルートシグネチャ生成
	hr = Engine::GetInstance()->Device()->CreateRootSignature(
		0, // GPUが複数ある場合のノードマスク（今回は1個しか無い想定なので0）
		pBlob->GetBufferPointer(), // シリアライズしたデータのポインタ
		pBlob->GetBufferSize(), // シリアライズしたデータのサイズ
		IID_PPV_ARGS(pRootSignature.GetAddressOf())); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}

void RootSignature::SetDistortionRootSignature()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[3] = {};
	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	// rootParam[1] : t0
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0; // t0
	descTblRange.RegisterSpace = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParam[1] : t1
	D3D12_DESCRIPTOR_RANGE descTblRange1 = {};
	descTblRange1.NumDescriptors = 1;
	descTblRange1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange1.BaseShaderRegister = 1; // t1
	descTblRange1.RegisterSpace = 0;
	descTblRange1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange1;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParam[2] : b0 (歪みのパラメータ)
	rootParam[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// ルートシグニチャの設定（設定したいルートパラメーターとスタティックサンプラーを入れる）
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam); // ルートパラメーターの個数をいれる
	desc.NumStaticSamplers = 1; // サンプラーの個数をいれる
	desc.pParameters = rootParam; // ルートパラメーターのポインタをいれる
	desc.pStaticSamplers = &samplerDesc; // サンプラーのポインタを入れる
	desc.Flags = flag; // フラグを設定

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
		return;
	}

	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ


	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}

// ポストエフェクト用 : t0, t1 の2枚を読む。サンプラーは LINEAR + CLAMP(ブルームなど、ぼかした画像を読む用)
void RootSignature::SetPostEffectTwoInputRootSignature()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // アプリケーションの入力アセンブラを使用する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; // ドメインシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; // ハルシェーダーのルートシグネチャへのアクセスを拒否する
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; // ジオメトリシェーダーのルートシグネチャへのアクセスを拒否する

	CD3DX12_ROOT_PARAMETER rootParam[3] = {};
	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//補間しない(ニアレストネイバー)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	// rootParam[1] : t0
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange.BaseShaderRegister = 0; // t0
	descTblRange.RegisterSpace = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParam[1] : t1
	D3D12_DESCRIPTOR_RANGE descTblRange1 = {};
	descTblRange1.NumDescriptors = 1;
	descTblRange1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange1.BaseShaderRegister = 1; // t1
	descTblRange1.RegisterSpace = 0;
	descTblRange1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange1;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParam[2] : b0 (歪みのパラメータ)
	rootParam[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// ルートシグニチャの設定（設定したいルートパラメーターとスタティックサンプラーを入れる）
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = std::size(rootParam); // ルートパラメーターの個数をいれる
	desc.NumStaticSamplers = 1; // サンプラーの個数をいれる
	desc.pParameters = rootParam; // ルートパラメーターのポインタをいれる
	desc.pStaticSamplers = &samplerDesc; // サンプラーのポインタを入れる
	desc.Flags = flag; // フラグを設定

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャシリアライズに失敗");
		OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
		return;
	}

	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗");
		return;
	}

	success = true;
}

void RootSignature::SetShadowDepthRootSignature()
{
    // ShadowDepthVSが使うb0(World/View/Proj)とb2(BoneBuffer)だけを定義する。
    D3D12_ROOT_PARAMETER rootParams[2]{};

    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0; // b0
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 2; // b2
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC desc{};
    desc.NumParameters = 2;
    desc.pParameters = rootParams;
    desc.NumStaticSamplers = 0;
    desc.pStaticSamplers = nullptr;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(
        &desc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        blob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr))
    {
        OutputDebugStringW(L"ShadowDepth RootSignatureのシリアライズに失敗\n");
        if (errorBlob.Get() != nullptr)
        {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        success = false;
        return;
    }

	hr = CreateRootSignatureCached(blob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ


    success = SUCCEEDED(hr);
    if (!success)
    {
        OutputDebugStringW(L"ShadowDepth RootSignatureの生成に失敗\n");
    }
}

void RootSignature::SetShadowDebugRootSignature()
{
    // t0: ShadowMap SRV、s0: 深度値を読むPoint Sampler。
    D3D12_DESCRIPTOR_RANGE srvRange{};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam.DescriptorTable.NumDescriptorRanges = 1;
    rootParam.DescriptorTable.pDescriptorRanges = &srvRange;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.MipLODBias = 0.0f;
    sampler.MaxAnisotropy = 1;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0; // s0
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC desc{};
    desc.NumParameters = 1;
    desc.pParameters = &rootParam;
    desc.NumStaticSamplers = 1;
    desc.pStaticSamplers = &sampler;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&desc,D3D_ROOT_SIGNATURE_VERSION_1_0,blob.GetAddressOf(),errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugStringW(L"ShadowDebug RootSignatureのシリアライズに失敗\n");
        if (errorBlob.Get() != nullptr)
        {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        success = false;
        return;
    }

	hr = CreateRootSignatureCached(blob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

    success = SUCCEEDED(hr);
    if (!success)
    {
        OutputDebugStringW(L"ShadowDebug RootSignatureの生成に失敗\n");
    }
}

void RootSignature::SetPBRRootSignature()
{
	// RootSignatureの設定
	D3D12_ROOT_SIGNATURE_FLAGS flag =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// RootParameter
	CD3DX12_ROOT_PARAMETER rootParam[15] = {};

	// RootParameter 0 : b0 Transform
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

	// RootParameter 1 : t0 MaterialTexture
	D3D12_DESCRIPTOR_RANGE materialRange = {};

	materialRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	materialRange.NumDescriptors = 1;
	materialRange.BaseShaderRegister = 0;
	materialRange.RegisterSpace = 0;
	materialRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[1].InitAsDescriptorTable(1, &materialRange, D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 2 : b1 Light
	rootParam[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 3 : b2 Bone
	rootParam[3].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);

	//RootParameter 4 : t1 ShadowMap
	D3D12_DESCRIPTOR_RANGE shadowMapRange = {};

	shadowMapRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	shadowMapRange.NumDescriptors = 1;
	shadowMapRange.BaseShaderRegister = 1;
	shadowMapRange.RegisterSpace = 0;
	shadowMapRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[4].InitAsDescriptorTable(1, &shadowMapRange, D3D12_SHADER_VISIBILITY_PIXEL);

	// RootParameter 5 : t2 NormalMap
	D3D12_DESCRIPTOR_RANGE normalRange = {};

	normalRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	normalRange.NumDescriptors = 1;
	normalRange.BaseShaderRegister = 2;
	normalRange.RegisterSpace = 0;
	normalRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[5].InitAsDescriptorTable(1, &normalRange, D3D12_SHADER_VISIBILITY_PIXEL);

	//RootParameter 3 : b3 Normal
	rootParam[6].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);
	//RootParameter 3 : b4 Specular
	rootParam[7].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL);
	//RootParameter 8 : b5 Bloom
	rootParam[8].InitAsConstantBufferView(5, 0, D3D12_SHADER_VISIBILITY_ALL);

	// RootParameter 5 : t2 NormalMap
	D3D12_DESCRIPTOR_RANGE heightRange = {};

	heightRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	heightRange.NumDescriptors = 1;
	heightRange.BaseShaderRegister = 3;
	heightRange.RegisterSpace = 0;
	heightRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[9].InitAsDescriptorTable(1, &heightRange, D3D12_SHADER_VISIBILITY_ALL);

	//RootParameter 9 : b6 Height
	rootParam[10].InitAsConstantBufferView(6, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParam[11].InitAsConstantBufferView(7, 0, D3D12_SHADER_VISIBILITY_ALL);

	// RootParameter 5 : t2 NormalMap
	D3D12_DESCRIPTOR_RANGE metallicRange = {};

	metallicRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	metallicRange.NumDescriptors = 1;
	metallicRange.BaseShaderRegister = 4;
	metallicRange.RegisterSpace = 0;
	metallicRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[12].InitAsDescriptorTable(1, &metallicRange, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_DESCRIPTOR_RANGE roughnessRange = {};

	roughnessRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	roughnessRange.NumDescriptors = 1;
	roughnessRange.BaseShaderRegister = 5;
	roughnessRange.RegisterSpace = 0;
	roughnessRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[13].InitAsDescriptorTable(1, &roughnessRange, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_DESCRIPTOR_RANGE aoRange = {};

	aoRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	aoRange.NumDescriptors = 1;
	aoRange.BaseShaderRegister = 6;
	aoRange.RegisterSpace = 0;
	aoRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParam[14].InitAsDescriptorTable(1, &aoRange, D3D12_SHADER_VISIBILITY_ALL);

	//Static Sampler : s0
	D3D12_STATIC_SAMPLER_DESC samplerDescs[5] = {};

	// Sampler 0 : Material Texture (s0)
	samplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[0].MipLODBias = 0.0f;
	samplerDescs[0].MaxAnisotropy = 1;
	samplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[0].MinLOD = 0.0f;
	samplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[0].ShaderRegister = 0;
	samplerDescs[0].RegisterSpace = 0;
	samplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Sampler 1 : ShadowMap (s1)
	samplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	samplerDescs[1].MipLODBias = 0.0f;
	samplerDescs[1].MaxAnisotropy = 1;
	samplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	samplerDescs[1].MinLOD = 0.0f;
	samplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[1].ShaderRegister = 1;
	samplerDescs[1].RegisterSpace = 0;
	samplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDescs[2] = samplerDescs[0];

	samplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// シャドウマップの範囲外は「影なし(深度1.0)」として扱う
	samplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	samplerDescs[2].MaxAnisotropy = 1;
	samplerDescs[2].ShaderRegister = 2;

	// Sampler 0 : Material Texture (s0)
	samplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	samplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[3].MipLODBias = 0.0f;
	samplerDescs[3].MaxAnisotropy = 1;
	samplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[3].MinLOD = 0.0f;
	samplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[3].ShaderRegister = 3;
	samplerDescs[3].RegisterSpace = 0;
	samplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Sampler 4 : HeightMap Texture (40)
	samplerDescs[4].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	samplerDescs[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	samplerDescs[4].MipLODBias = 0.0f;
	samplerDescs[4].MaxAnisotropy = 1;
	samplerDescs[4].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[4].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDescs[4].MinLOD = 0.0f;
	samplerDescs[4].MaxLOD = D3D12_FLOAT32_MAX;

	samplerDescs[4].ShaderRegister = 4;
	samplerDescs[4].RegisterSpace = 0;
	samplerDescs[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.NumParameters = _countof(rootParam);
	desc.pParameters = rootParam;

	desc.NumStaticSamplers = _countof(samplerDescs);
	desc.pStaticSamplers = samplerDescs;

	desc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}

		OutputDebugStringW(L"RootSignatureのシリアライズに失敗");

		success = false;
		return;
	}

	hr = CreateRootSignatureCached(pBlob.Get(), pRootSignature); // ルートシグニチャ格納先のポインタ

	if (FAILED(hr))
	{
		OutputDebugStringW(L"RootSignatureの生成に失敗");

		success = false;
		return;
	}

	success = true;
}

// ============================================================================
//  ルートシグネチャ共有まわりの公開関数
// ============================================================================
bool RootSignature::IsShared(const ID3D12RootSignature * _rootSignature)
{
	if (_rootSignature == nullptr)
	{
		return false;
	}
	
	std::lock_guard<std::mutex> lock(rootSignatureMutex);
	return rootSignatureSet.count(_rootSignature) != 0;
}

void RootSignature::ClearShared()
{
	std::lock_guard<std::mutex> lock(rootSignatureMutex);
	rootSignatureSet.clear();
	rootSignatureCache.clear();
}

// ---- 互換用(FBXModel が使っている)。中身が同じなら実体は共有されるので、毎回作っても同じ ----
std::shared_ptr<RootSignature> RootSignature::GetSharedModel()
{
	auto rootSignature = std::make_shared<RootSignature>();
	rootSignature->SetModelRootSignature();
	return rootSignature;
}

std::shared_ptr<RootSignature> RootSignature::GetSharedPBR()
{
	auto rootSignature = std::make_shared<RootSignature>();
	rootSignature->SetPBRRootSignature();
	return rootSignature;
}

std::shared_ptr<RootSignature> RootSignature::GetSharedShadowDepth()
{
	auto rootSignature = std::make_shared<RootSignature>();
	rootSignature->SetShadowDepthRootSignature();
	return rootSignature;
}