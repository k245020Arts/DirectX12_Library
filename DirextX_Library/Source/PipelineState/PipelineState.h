#pragma once
#include "../ComPtr.h"
#include "../../DirectX12_Library/d3dx12.h"
#include <string>
#include "../ShaderStruct/ShaderStruct.h"

class PipelineState
{
public:
	PipelineState();
	PipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE _desc); // コンストラクタである程度の設定をする
	bool IsSuccess(); // 生成に成功したかどうかを返す

	void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& _layout); // 入力レイアウトを設定
	void SetRootSignature(ID3D12RootSignature* _rootSignature); // ルートシグネチャを設定
	void SetVS(std::wstring _filePath,std::string _entryPoint); // 頂点シェーダーを設定
	void SetPS(std::wstring _filePath, std::string _entryPoint); // ピクセルシェーダーを設定
	void Create(); // パイプラインステートを生成
	

	void SetDrawLayOut(D3D12_FILL_MODE _fillMode);
	void SetBlendMode(BlendState _state);

	ID3D12PipelineState* GetPipelineState();

private:
	bool success = false; // 生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {}; // パイプラインステートの設定
	ComPtr<ID3D12PipelineState> defalutPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> alphaPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> addPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> mulPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> subPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3D12PipelineState> currentPipelineState = nullptr; // パイプラインステート
	ComPtr<ID3DBlob> pVSBlob; // 頂点シェーダー
	ComPtr<ID3DBlob> pPSBlob; // ピクセルシェーダー
	ComPtr<ID3DBlob> errorBlob; // エラー検出用

	BlendState blendState;

	void CreateAlphaPipelineState(); // パイプラインステートを生成
	void CreateMulPipelineState(); // パイプラインステートを生成
	void CreateAddPipelineState(); // パイプラインステートを生成
	void CreateSubPipelineState(); // パイプラインステートを生成
};

