#pragma once
#include "../ComPtr.h"
#include "../../DirectX12_Library/d3dx12.h"

struct ID3D12RootSignature;

class RootSignature
{
public:
	RootSignature(); // コンストラクタでルートシグネチャを生成
	bool IsSuccess(); // ルートシグネチャの生成に成功したかどうかを返す
	ID3D12RootSignature* Get(); // ルートシグネチャを返す

private:
	bool success = false; // ルートシグネチャの生成に成功したかどうか
	ComPtr<ID3D12RootSignature> pRootSignature = nullptr; // ルートシグネチャ
};


