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
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // 塗りつぶし
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // ブレンドステートもデフォルト
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = _desc; //指定した形を描画
	desc.NumRenderTargets = 1; // 描画対象は1
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1; // サンプラーは1
	desc.SampleDesc.Quality = 0;

	blendState = BlendState::NO_BLEND;
}

bool PipelineState::IsSuccess()
{
	return success;
}

void PipelineState::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layout)
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
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(defalutPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}

	CreateMulPipelineState();
	CreateAddPipelineState();
	CreateSubPipelineState();
	CreateAlphaPipelineState();

	currentPipelineState = defalutPipelineState;
	success = true;
}

void PipelineState::SetDrawLayOut(D3D12_FILL_MODE _fillMode)
{
	desc.RasterizerState.FillMode = _fillMode;
}

void PipelineState::SetBlendMode(BlendState _state)
{
	switch (_state)
	{
	case BlendState::NO_BLEND:
		currentPipelineState = defalutPipelineState;
		break;
	case BlendState::ALPHA:
		currentPipelineState = alphaPipelineState;
		break;
	case BlendState::ADD:
		currentPipelineState = addPipelineState;
		break;
	case BlendState::SUB:
		currentPipelineState = subPipelineState;
		break;
	case BlendState::MUL:
		currentPipelineState = mulPipelineState;
		break;
	default:
		assert(false && "こちらのブレンドはありません");
		break;
	}

	blendState = _state;
}

ID3D12PipelineState* PipelineState::GetPipelineState()
{
	return currentPipelineState.Get();
}

void PipelineState::CreateAlphaPipelineState()
{
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	//Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	desc.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	desc.BlendState.IndependentBlendEnable = false;
	desc.BlendState.AlphaToCoverageEnable = false;

	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(alphaPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}

void PipelineState::CreateMulPipelineState()
{
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_SRC_COLOR;

	//Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	desc.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	desc.BlendState.IndependentBlendEnable = false;
	desc.BlendState.AlphaToCoverageEnable = false;

	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(mulPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}

void PipelineState::CreateAddPipelineState()
{
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;

	//Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	desc.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	desc.BlendState.IndependentBlendEnable = false;
	desc.BlendState.AlphaToCoverageEnable = false;

	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(addPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}

void PipelineState::CreateSubPipelineState()
{
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(subPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}
