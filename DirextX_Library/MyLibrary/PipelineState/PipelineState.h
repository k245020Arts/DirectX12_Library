#pragma once
#include "../ComPtr.h"
#include "../../DirectX12_Library/d3dx12.h"
#include <string>
#include "../ShaderStruct/ShaderStruct.h"

class PipelineState
{
public:
	void ClearChche();
	PipelineState();
	~PipelineState() {};
	PipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE _desc); // コンストラクタである程度の設定をする
	bool IsSuccess(); // 生成に成功したかどうかを返す

	void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& _layout); // 入力レイアウトを設定
	void SetRootSignature(ID3D12RootSignature* _rootSignature); // ルートシグネチャを設定
	void SetVS(std::wstring _filePath,std::string _entryPoint); // 頂点シェーダーを設定
	void SetPS(std::wstring _filePath, std::string _entryPoint); // ピクセルシェーダーを設定
	void Create(); // パイプラインステートを生成
	void PostEffectPipelineCreate();
	void CreateDepthOnly();
	
	void ErrorStateChange();
	void SetDrawLayOut(D3D12_FILL_MODE _fillMode);
	void SetBlendMode(BlendState _state);

	ID3D12PipelineState* GetPipelineState();

	void SetDepthEnable(bool enable);
	void SetDepthWriteMask(D3D12_DEPTH_WRITE_MASK mask);
	void SetDepthFunc(D3D12_COMPARISON_FUNC func);

	// 描画先が複数ある(マルチレンダーターゲット)ときの設定。Create の前に呼ぶ
	//  _formats : 描画先ごとのフォーマット(バインドするターゲットと同じ並びにすること)
	void SetRenderTargetFormats(const DXGI_FORMAT* _formats, UINT _count);

	// ピクセルシェーダーが 2枚目以降のターゲット(SV_TARGET1~)にも出力するかどうか
	//  false(既定) : 2枚目以降には書き込まない(中身はそのまま残る)
	//  true        : ピクセルシェーダーの出力を書き込む(シェーダー側で SV_TARGET1~ を出力しておくこと)
	void SetExtraTargetOutput(bool _enable) { extraTargetOutput = _enable; }

private:
	bool success = false; // 生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {}; // パイプラインステートの設定
	ComPtr<ID3D12PipelineState> defalutPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> alphaPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> addPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> mulPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> subPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> currentPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> errorPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3DBlob> pVSBlob; // 頂点シェーダー
	ComPtr<ID3DBlob> pPSBlob; // ピクセルシェーダー
	ComPtr<ID3DBlob> errorBlob; // エラー検出用

	BlendState blendState;

	UINT renderTargetCount = 1;       // 描画先の数(desc.NumRenderTargets と同じ)
	bool extraTargetOutput = false;   // 2枚目以降のターゲットへ書き込むか
	void ApplyExtraTargetBlend();     // 2枚目以降のターゲットのブレンド設定(書き込みマスク)を desc に反映

	void CreateAlphaPipelineState(); // パイプラインステートを生成
	void CreateMulPipelineState(); // パイプラインステートを生成
	void CreateAddPipelineState(); // パイプラインステートを生成
	void CreateSubPipelineState(); // パイプラインステートを生成
	void CreateErrorPipelineState();
};

