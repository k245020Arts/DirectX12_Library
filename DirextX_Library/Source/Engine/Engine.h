#pragma once
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

#include "../Window/Window.h"
#include "../Comptr.h"
#include "../SingleTon/SingletonBase.h"

static constexpr int FRAME_BUFFER_COUNT = 2;

class Engine : public SingletonBase<Engine>
{
public:


	~Engine();

	bool Init(const HWND _hwnd, const Size& _size); // エンジン初期化

	void BeginRender();
	void EndRender();
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	UINT CurrentBackBufferIndex();

	const Size& GetWindowSize() { return windowSize; }

private:

	void CreateDebugLayer(); //デバックレイヤーの生成
	bool CreateDevice(); // デバイスを生成
	bool CreateAdapter(); //アダプターの生成
	bool CreateCommandQueue(); // コマンドキューを生成
	bool CreateSwapChain(); // スワップチェインを生成
	bool CreateCommandList(); // コマンドリストとコマンドアロケーターを生成
	bool CreateFence(); // フェンスを生成
	void CreateViewPort(); // ビューポートを生成
	void CreateScissorRect(); // シザー矩形を生成

	HWND hwnd;
	Size windowSize = Size();
	UINT currentBackBufferIndex = 0;

	ComPtr<ID3D12Debug> debugController = nullptr;
	ComPtr<ID3D12Device6> pDevice = nullptr; 
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;
	ComPtr<IDXGIFactory6> dxgiFactory = nullptr; //アダプターの列挙をするためのオブジェクト1
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr; 
	ComPtr<IDXGISwapChain3> swapChain = nullptr; 
	std::vector<ComPtr<ID3D12CommandAllocator>> commandAllocator = { nullptr };
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;
	UINT64 m_fenceValue = 0;
	D3D12_VIEWPORT viewport; 
	D3D12_RECT scissor;

	bool CreateRenderTarget();
	bool CreateDepthBuffer();

	UINT m_RtvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> pRtvHeap = nullptr;
	std::vector<ComPtr<ID3D12Resource>> pRenderTargets = { nullptr };

	UINT m_DsvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> pDsvHeap = nullptr;
	ComPtr<ID3D12Resource> pDepthStencilBuffer = nullptr;

	ID3D12Resource* currentRenderTarget = nullptr;

	void WaitRender();

	friend class SingletonBase<Engine>;
};
