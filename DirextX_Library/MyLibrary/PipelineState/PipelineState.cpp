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
		// どのファイルで失敗したか、HRESULT はいくつかを出す
		std::wstring msg = L"頂点シェーダーの読み込みに失敗: " + filePath
			+ L" hr=0x" + std::to_wstring(static_cast<unsigned long>(hr)) + L"\n";
		OutputDebugStringW(msg.c_str());

		// HLSL のコンパイルエラーなら、行番号つきの内容がここに入っている
		if (errorBlob)
		{
			OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
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
		// どのファイルで失敗したか、HRESULT はいくつかを出す
		std::wstring msg = L"ピクセルシェーダーの読み込みに失敗: " + filePath + L" hr=0x" + std::to_wstring(static_cast<unsigned long>(hr)) + L"\n";
		OutputDebugStringW(msg.c_str());

		// HLSL のコンパイルエラーなら、行番号つきの内容がここに入っている
		if (errorBlob)
		{
			OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		return;
	}

	desc.PS = CD3DX12_SHADER_BYTECODE(pPSBlob.Get());
}

void PipelineState::Create()
{
	// マルチレンダーターゲットのとき、2枚目以降の書き込み設定を反映する(描画先が1枚なら何も変わらない)
	ApplyExtraTargetBlend();

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
	CreateErrorPipelineState();

	currentPipelineState = defalutPipelineState;
	success = true;
}

void PipelineState::PostEffectPipelineCreate()
{
	// パイプラインステートを生成
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(defalutPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}

	currentPipelineState = defalutPipelineState;
	success = true;
}

void PipelineState::CreateDepthOnly()
{
	// カラーレンダーターゲットを使用しない
	desc.NumRenderTargets = 0;
	desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;

	// 深度バッファへの書き込みを有効化
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	desc.DepthStencilState.DepthEnable = TRUE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	desc.RasterizerState.DepthBias = 0;
	desc.RasterizerState.SlopeScaledDepthBias = 2.0f;   // 1.5〜3.0で調整
	desc.RasterizerState.DepthBiasClamp = 0.0f;

	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	// ピクセルシェーダーは使用しない
	desc.PS = {};

	// 深度専用PSOを生成
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc,IID_PPV_ARGS(defalutPipelineState.ReleaseAndGetAddressOf()));

	if (FAILED(hr))
	{
		OutputDebugStringW(L"深度専用パイプラインステートの生成に失敗\n");
		return;
	}

	currentPipelineState = defalutPipelineState;
	success = true;
}

void PipelineState::ErrorStateChange()
{
	currentPipelineState = errorPipelineState;
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

void PipelineState::SetDepthEnable(bool enable)
{
	desc.DepthStencilState.DepthEnable = enable;
}

void PipelineState::SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK mask)
{
	desc.DepthStencilState.DepthWriteMask = mask;
}

void PipelineState::SetDepthFunc(D3D12_COMPARISON_FUNC func)
{
	desc.DepthStencilState.DepthFunc = func;
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
	ApplyExtraTargetBlend(); // 2枚目以降のターゲット(MRT)の設定
	desc.BlendState.AlphaToCoverageEnable = false;
	//半透明用に、深度バッファへの書き込みだけを「禁止（ZERO）」にする
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthEnable = true;

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
	ApplyExtraTargetBlend(); // 2枚目以降のターゲット(MRT)の設定
	desc.BlendState.AlphaToCoverageEnable = false;
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う

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
	ApplyExtraTargetBlend(); // 2枚目以降のターゲット(MRT)の設定
	desc.BlendState.AlphaToCoverageEnable = false;
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う

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

void PipelineState::CreateErrorPipelineState()
{
	// エラー用ピクセルシェーダーは SV_TARGET0 にしか出力しないので、2枚目以降には書き込まない
	const bool prevExtraOutput = extraTargetOutput;
	extraTargetOutput = false;
	ApplyExtraTargetBlend();
	extraTargetOutput = prevExtraOutput;

	SetPS(L"Shader/ErrorShader/ErrorPixelShader.hlsl", "ErrorMain");
	auto hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(errorPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}

void PipelineState::SetRenderTargetFormats(const DXGI_FORMAT* _formats, UINT _count)
{
	assert(_formats != nullptr);
	assert(_count >= 1 && _count <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);

	desc.NumRenderTargets = _count;

	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		desc.RTVFormats[i] = (i < _count) ? _formats[i] : DXGI_FORMAT_UNKNOWN;
	}

	renderTargetCount = _count;
}

void PipelineState::ApplyExtraTargetBlend()
{
	// 描画先が1枚なら従来のまま(RenderTarget[0] の設定が使われる)
	if (renderTargetCount <= 1)
	{
		return;
	}

	// ターゲットごとに別々の設定にする。RenderTarget[0] は各ブレンドモードの設定がそのまま使われる
	desc.BlendState.IndependentBlendEnable = TRUE;

	for (UINT i = 1; i < renderTargetCount; ++i)
	{
		D3D12_RENDER_TARGET_BLEND_DESC extra = {};

		// 2枚目以降はブレンドしない(光らせたい色などは、そのまま上書きしたい)
		extra.BlendEnable = FALSE;
		extra.LogicOpEnable = FALSE;
		extra.SrcBlend = D3D12_BLEND_ONE;
		extra.DestBlend = D3D12_BLEND_ZERO;
		extra.BlendOp = D3D12_BLEND_OP_ADD;
		extra.SrcBlendAlpha = D3D12_BLEND_ONE;
		extra.DestBlendAlpha = D3D12_BLEND_ZERO;
		extra.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		extra.LogicOp = D3D12_LOGIC_OP_NOOP;

		// 出力しないシェーダーのときは書き込みマスクを 0 にして、ターゲットの中身を変えない
		extra.RenderTargetWriteMask = extraTargetOutput ? D3D12_COLOR_WRITE_ENABLE_ALL : 0;

		desc.BlendState.RenderTarget[i] = extra;
	}
}
