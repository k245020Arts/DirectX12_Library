#include "PipelineState.h"
#include "../Engine/Engine.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

PipelineState::PipelineState() : PipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
{
	
}

PipelineState::PipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE _desc)
{
	// パイプラインステートの設定
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // ラスタライザーはデフォルト
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // カリングはなし
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // ブレンドステートもデフォルト
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = _desc; //指定した形を描画
	desc.NumRenderTargets = 1; // 描画対象は1
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1; // サンプラーは1
	desc.SampleDesc.Quality = 0;
}

bool PipelineState::IsSuccess()
{
	return success;
}

void PipelineState::SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout)
{
	desc.InputLayout = layout;
}

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	desc.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath, std::string _entryPoint)
{
	// 頂点シェーダー読み込み
	auto hr = D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		_entryPoint.c_str(), "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pVSBlob, &errorBlob);

	if (FAILED(hr))
	{
		printf("頂点シェーダーの読み込みに失敗");
		return;
	}

	desc.VS = CD3DX12_SHADER_BYTECODE(pVSBlob.Get());
}

void PipelineState::SetPS(std::wstring filePath, std::string _entryPoint)
{
	// ピクセルシェーダー読み込み
	auto hr = D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		_entryPoint.c_str(), "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pPSBlob, &errorBlob);

	if (FAILED(hr))
	{
		OutputDebugStringW(L"ピクセルシェーダーの読み込みに失敗");
		return;
	}

	desc.PS = CD3DX12_SHADER_BYTECODE(pPSBlob.Get());
}

void PipelineState::Create()
{
	// パイプラインステートを生成
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}

	success = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return pipelineState.Get();
}
