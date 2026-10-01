#pragma once
#include "../ComPtr.h"
#include "../../DirectX12_Library/d3dx12.h"
#include <string>

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

	ID3D12PipelineState* GetPipelineState();
	ID3D12PipelineState* GetPipelineStateWireFrame();

private:
	bool success = false; // 生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {}; // パイプラインステートの設定
	ComPtr<ID3D12PipelineState> pipelineState = nullptr; // パイプラインステート(塗りつぶし表示用)
	ComPtr<ID3D12PipelineState> pipelineStateWireframe = nullptr; // パイプラインステート(ワイヤーフレーム表示用)
	ComPtr<ID3DBlob> pVSBlob; // 頂点シェーダー
	ComPtr<ID3DBlob> pPSBlob; // ピクセルシェーダー
	ComPtr<ID3DBlob> errorBlob; // エラー検出用
};

