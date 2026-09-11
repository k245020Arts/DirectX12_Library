#pragma once
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include<wrl.h>
#include <DirectXTex.h>
#include "../../DirectX12_Library/d3dx12.h"

//ライブラリのリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

#include <Windows.h>
#include <vector>

#include <DirectXMath.h>


using Microsoft::WRL::ComPtr;

struct Vertex
{
	DirectX::XMFLOAT3 pos; //座標
	DirectX::XMFLOAT2 uv; //uv座標
};

struct MatriceData
{
	DirectX::XMMATRIX world; //モデル本体を回転させたり移動させたりする行列
	DirectX::XMMATRIX viewproj; //ビューとプロジェクション合成行列
};

struct PMDVertex
{
	DirectX::XMFLOAT3 pos;//頂点座標 : 12バイト
	DirectX::XMFLOAT3 normal;//法線ベクトル : 12バイト
	DirectX::XMFLOAT2 uv; //uv座標 : 8バイト
	unsigned short boneNo[2]; //ボーン番号 : 4バイト
	unsigned char boneWeight;//ボーン影響度 : 1バイト
	unsigned char edgeFlg; //輪郭線フラグ : 1バイト
	unsigned short dummy; //ダミーがないと38バイトで終わってしまうので、ダミーを入れて40バイトになるようにする

	PMDVertex()
	{
		pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		boneNo[0] = 0;
		boneNo[1] = 0;
		boneWeight = 0;
		edgeFlg = 0;
		dummy = 0;
	}
};

int AlignmentedSize(size_t size, size_t alignment);

class Window
{
public:
	~Window();
	///<summary>
	///	ウィンドウの作成
	///	< / summary>
	///	<param name = "clientWidth">#< / param>
	///	<param name = "clientHeight"> = < / param>
	///	<param name = "titleName"> < l & < / param>
	///	<param name = "windowClassName">< / param>
	bool Create(int _cWidth,int _cHeight,const std::wstring& _titleName,const std::wstring& _windowClassName);

	bool ProcessMessage();

	void Update();

	bool ScreenFlip();

private:

	ComPtr<ID3D12Device> _dev = nullptr; //デバイスのオブジェクトのポインタ
	ComPtr <IDXGIFactory6> _dxgiFactory = nullptr; //アダプターの列挙をするためのオブジェクト1
	ComPtr <IDXGISwapChain4> _swapChain = nullptr;

	ComPtr <ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr <ID3D12GraphicsCommandList> _cmdList = nullptr;

	//フェンスの作成
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;

	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr;

	//コマンドキューの設定
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;

	std::vector<ComPtr<ID3D12Resource>> _backBuffers;

	float plus = 0;

	ComPtr<ID3D12Resource> vertBuff = nullptr;
	ComPtr<ID3D12Resource> idxBuff = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	//ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	D3D12_VIEWPORT viewport = {};

	D3D12_RECT scissorrect = {};

	ComPtr<ID3D12Resource> texbuff = nullptr;

	ComPtr<ID3D12Resource> uploadBuff = nullptr;

	ComPtr <ID3D12RootSignature> rootsignature = nullptr;

	ComPtr <ID3D12DescriptorHeap> basicDescHeap = nullptr;

	ComPtr <ID3DBlob> rootSigBlob = nullptr;

	ComPtr<ID3D12Resource> constBuff = nullptr;

	MatriceData* mapMatrix;
	float angle;

	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;

	unsigned int vertNum;
	unsigned int indicsNum;

	ComPtr <ID3D12Resource> depthBuffer = nullptr;
	ComPtr <ID3D12DescriptorHeap> dsvHeap = nullptr;

};