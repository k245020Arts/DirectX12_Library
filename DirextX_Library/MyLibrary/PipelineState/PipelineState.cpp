#include "PipelineState.h"
#include "../Engine/Engine.h"
#include <d3dcompiler.h>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "../RootSignature/RootSignature.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
	constexpr UINT shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	//シェーダーのコンパイル結果のキャッシュ

	std::mutex shaderCacheMutex;
	std::unordered_map<std::wstring, ComPtr<ID3D10Blob>> shaderCache;

	bool CompileShaderCached(const std::wstring& _path, const std::string& _entry, const char* _target, ComPtr<ID3D10Blob>& _outBlob, ComPtr<ID3D10Blob>& _errorBlob, HRESULT _outHr)
	{
		std::wstring key = _path;
		key += L'|';
		key.append(_entry.begin(), _entry.end());
		key += L'|';
		key.append(_target, _target + std::strlen(_target));
		{
			std::lock_guard<std::mutex> lock(shaderCacheMutex);
			auto it = shaderCache.find(key);
			if (it != shaderCache.end())
			{
				_outBlob = it->second;
				_outHr = S_OK;
				return true;
			}
			
		}

		ComPtr<ID3DBlob> blob;
		_outHr = D3DCompileFromFile(_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, _entry.c_str(), _target, shaderCompileFlags, 0, blob.GetAddressOf(), _errorBlob.ReleaseAndGetAddressOf());
		
		if (FAILED(_outHr))
		{
			_outBlob.Reset();
			return false; // 失敗はキャッシュしない(直して再実行すれば、また試せる)
		}
		{
			std::lock_guard<std::mutex> lock(shaderCacheMutex);
			auto result = shaderCache.emplace(key, blob); // 同時に入った場合は、先に入った方を使う
			_outBlob = result.first->second;
		}
		return true;
	}

	//PSOのキャッシュ

	struct PSOCacheEntry
	{
		ComPtr <ID3D12PipelineState> pso;
		ComPtr<ID3D12RootSignature> rootSignature;
	};

	std::mutex psoCacheMutex;
	std::unordered_map<std::string, PSOCacheEntry> psoCache;

	template<typename T>
	void Append(std::string& _key, const T& _value)
	{
		//memory_copyが出来ない値はコンパイル時にはじく
		static_assert(std::is_trivially_copyable<T>::value, "Appendは単純な値だけに使う");
		_key.append((const char*)&_value, sizeof(T));
	}

	uint64_t HashBytes(const void* _data, size_t _size)
	{
		const uint8_t * p = static_cast<const uint8_t*>(_data);
		uint64_t h = 14695981039346656037ull;
		for (size_t i = 0; i < _size; ++i)
		{
			h ^= p[i];
			h *= 1099511628211ull;
		}
		return h;
	}

	void AppendShader(std::string& _key, const D3D12_SHADER_BYTECODE& _s)
	{
		 Append(_key, static_cast<uint64_t>(_s.BytecodeLength));
		 Append(_key, _s.BytecodeLength ? HashBytes(_s.pShaderBytecode, _s.BytecodeLength) : 0ull);
	}
	 
	// 構造体のパディングに入るゴミで一致しなくならないよう、メンバーを1つずつ追加する
	void AppendRenderTargetBlend(std::string & _key, const D3D12_RENDER_TARGET_BLEND_DESC & _b)
	{
		 Append(_key, _b.BlendEnable);
		 Append(_key, _b.LogicOpEnable);
		 Append(_key, _b.SrcBlend);
		 Append(_key, _b.DestBlend);
		 Append(_key, _b.BlendOp);
		 Append(_key, _b.SrcBlendAlpha);
		 Append(_key, _b.DestBlendAlpha);
		 Append(_key, _b.BlendOpAlpha);
		 Append(_key, _b.LogicOp);
		 Append(_key, _b.RenderTargetWriteMask);
	}
	 
	void AppendStencilOp(std::string & _key, const D3D12_DEPTH_STENCILOP_DESC & _o)
	{
		 Append(_key, _o.StencilFailOp);
		 Append(_key, _o.StencilDepthFailOp);
		 Append(_key, _o.StencilPassOp);
		 Append(_key, _o.StencilFunc);
	}
	 
	std::string BuildPsoKey(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & _d)
	{
		 std::string key;
		 key.reserve(512);
		 
		Append(key, reinterpret_cast<uintptr_t>(_d.pRootSignature));
		AppendShader(key, _d.VS);
		AppendShader(key, _d.PS);
		 
			 	// ブレンド
		Append(key, _d.BlendState.AlphaToCoverageEnable);
		Append(key, _d.BlendState.IndependentBlendEnable);
		for (const auto& rt : _d.BlendState.RenderTarget)
		{
			AppendRenderTargetBlend(key, rt);
		}
		 
		Append(key, _d.SampleMask);
		 
		// ラスタライザー
		const D3D12_RASTERIZER_DESC & r = _d.RasterizerState;
		Append(key, r.FillMode);
		Append(key, r.CullMode);
		Append(key, r.FrontCounterClockwise);
		Append(key, r.DepthBias);
		Append(key, r.DepthBiasClamp);
		Append(key, r.SlopeScaledDepthBias);
		Append(key, r.DepthClipEnable);
		Append(key, r.MultisampleEnable);
		Append(key, r.AntialiasedLineEnable);
		Append(key, r.ForcedSampleCount);
		Append(key, r.ConservativeRaster);
		 
		// 深度・ステンシル
		const D3D12_DEPTH_STENCIL_DESC & ds = _d.DepthStencilState;
		Append(key, ds.DepthEnable);
		Append(key, ds.DepthWriteMask);
		Append(key, ds.DepthFunc);
		Append(key, ds.StencilEnable);
		Append(key, ds.StencilReadMask);
		Append(key, ds.StencilWriteMask);
		AppendStencilOp(key, ds.FrontFace);
		AppendStencilOp(key, ds.BackFace);
		 
		// 入力レイアウト(セマンティクス名は文字列の中身で比べる)
		Append(key, _d.InputLayout.NumElements);
		for (UINT i = 0; i < _d.InputLayout.NumElements; ++i)
		{
			const D3D12_INPUT_ELEMENT_DESC & e = _d.InputLayout.pInputElementDescs[i];
			key.append(e.SemanticName ? e.SemanticName : "");
			key.push_back('\0');
			Append(key, e.SemanticIndex);
			Append(key, e.Format);
			Append(key, e.InputSlot);
			Append(key, e.AlignedByteOffset);
			Append(key, e.InputSlotClass);
			Append(key, e.InstanceDataStepRate);
		}
		 
		Append(key, _d.IBStripCutValue);
		Append(key, _d.PrimitiveTopologyType);
		Append(key, _d.NumRenderTargets);
		for (const auto& f : _d.RTVFormats)
		{
			Append(key, f);
		}
		 Append(key, _d.DSVFormat);
		 Append(key, _d.SampleDesc.Count);
		 Append(key, _d.SampleDesc.Quality);
		 Append(key, _d.NodeMask);
		 Append(key, _d.Flags);
		 
		return key;
	}
	 
		 	// CreateGraphicsPipelineState の代わりに使う。失敗したら nullptr
	ComPtr<ID3D12PipelineState> CreateGraphicsPipelineStateCached(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & _desc)
	{
		 // 共有ルートシグネチャを使っていないもの(2D・ポストエフェクトなど)は、従来どおり毎回作る
		// (モデルごとに別のルートシグネチャを持つものまでキャッシュすると、増える一方になるため)
		// GS/HS/DS・ストリーム出力・キャッシュ済みPSOを使うものもキーに含めていないので対象外
		const bool cacheable =
			 RootSignature::IsShared(_desc.pRootSignature) &&
			 _desc.GS.BytecodeLength == 0 &&
			 _desc.HS.BytecodeLength == 0 &&
			 _desc.DS.BytecodeLength == 0 &&
			 _desc.StreamOutput.NumEntries == 0 &&
			 _desc.CachedPSO.CachedBlobSizeInBytes == 0;
		 
		std::string key;
		if (cacheable)
		{
			key = BuildPsoKey(_desc);
			 
			std::lock_guard<std::mutex> lock(psoCacheMutex);
			auto it = psoCache.find(key);
			if (it != psoCache.end())
			{
				return it->second.pso;
			}
		}
		 
		// 時間のかかる生成はロックの外で行う
		ComPtr<ID3D12PipelineState> pso;
		const HRESULT hr = Engine::GetInstance()->Device()->CreateGraphicsPipelineState(&_desc, IID_PPV_ARGS(pso.GetAddressOf()));
		if (FAILED(hr))
		{
			return nullptr;
		}
		 
		if (cacheable)
		{
			 PSOCacheEntry entry;
			 entry.pso = pso;
			 entry.rootSignature = _desc.pRootSignature;
			 
			std::lock_guard<std::mutex> lock(psoCacheMutex);
			auto result = psoCache.emplace(std::move(key), std::move(entry)); // 同時に作られていたら、先に入った方を使う
			return result.first->second.pso;
		}
		 
		return pso;
	}
}

void PipelineState::ClearChche()
{
	std::lock_guard<std::mutex> psolock(psoCacheMutex);
	psoCache.clear();
	std::lock_guard<std::mutex> shaderLock(shaderCacheMutex);
	shaderCache.clear();

}

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
	HRESULT hr = S_OK;
	const bool compiled = CompileShaderCached(filePath, _entryPoint, "vs_5_0", pVSBlob, errorBlob, hr);

	if (!compiled)
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
	// 頂点シェーダー読み込み
	HRESULT hr = S_OK;
	const bool compiled = CompileShaderCached(filePath, _entryPoint, "ps_5_0", pPSBlob, errorBlob, hr);

	if (!compiled)
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

	defalutPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!defalutPipelineState)
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
	defalutPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!defalutPipelineState)
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

	defalutPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!defalutPipelineState)
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

	alphaPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!alphaPipelineState)
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

	mulPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!mulPipelineState)
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

	addPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!addPipelineState)
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗");
		return;
	}
}

void PipelineState::CreateSubPipelineState()
{
	subPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!subPipelineState)
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
	errorPipelineState = CreateGraphicsPipelineStateCached(desc);
	if (!errorPipelineState)
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
