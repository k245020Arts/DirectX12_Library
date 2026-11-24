#pragma once
#include "../ComPtr.h"
#include "../../DirectX12_Library/d3dx12.h"
#include <memory>

struct ID3D12RootSignature;

class RootSignature
{
public:
	RootSignature(); // コンストラクタでルートシグネチャを生成
	bool IsSuccess(); // ルートシグネチャの生成に成功したかどうかを返す
	ID3D12RootSignature* Get(); // ルートシグネチャを返す

	///----------------------------------ルートシグネチャの設定---------------------------------
	void SetTextureRootSampler();
	void SetNormalRootSignature();
	void SetModelRootSignature();
	void Set3DRootSignature();
	void SetPostEffectRootSignature();
	void SetDistortionRootSignature();
	void SetPostEffectTwoInputRootSignature(); // t0, t1, b0 (サンプラーは LINEAR + CLAMP)
	void SetShadowDepthRootSignature();
	void SetShadowDebugRootSignature();
	void SetPBRRootSignature();

	///-------------------------------------------------------------------

	// ---- 全モデルで共有するルートシグネチャ(最初の1回だけ作り、以降は同じものを返す) ----
	// PSO のキャッシュは、ここで作ったルートシグネチャを使うものだけが対象になる
	static bool IsShared(const ID3D12RootSignature* _rootSignature); // キャッシュに登録されたものか
	static void ClearShared(); // キャッシュを破棄する。デバイスを解放する前に1回呼ぶこと
	
	// 互換用(FBXModel が使っている)。呼ぶたびに RootSignature は作るが、実体は共有される
	static std::shared_ptr<RootSignature> GetSharedModel();
	static std::shared_ptr<RootSignature> GetSharedPBR();
	static std::shared_ptr<RootSignature> GetSharedShadowDepth();

private:
	bool success = false; // ルートシグネチャの生成に成功したかどうか
	ComPtr<ID3D12RootSignature> pRootSignature = nullptr; // ルートシグネチャ
};


