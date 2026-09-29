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

using Microsoft::WRL::ComPtr;

static constexpr int FRAME_BUFFER_COUNT = 2;

class Engine
{
public:
	bool Init(const HWND _hwnd, const Size& _size); // エンジン初期化

	void BeginRender();
	void EndRender();
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	UINT CurrentBackBufferIndex();

private:

	bool CreateDevice(); // デバイスを生成
	bool CreateCommandQueue(); // コマンドキューを生成
	bool CreateSwapChain(); // スワップチェインを生成
	bool CreateCommandList(); // コマンドリストとコマンドアロケーターを生成
	bool CreateFence(); // フェンスを生成
	void CreateViewPort(); // ビューポートを生成
	void CreateScissorRect(); // シザー矩形を生成

	HWND hwnd;
	Size windowSize = Size();
	UINT m_CurrentBackBufferIndex = 0;

	ComPtr<ID3D12Device6> pDevice = nullptr; 
	ComPtr<ID3D12CommandQueue> pQueue = nullptr; 
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr; 
	ComPtr<ID3D12CommandAllocator> m_pAllocator[FRAME_BUFFER_COUNT] = { nullptr }; 
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> m_pFence = nullptr;
	UINT64 m_fenceValue[FRAME_BUFFER_COUNT];
	D3D12_VIEWPORT m_Viewport; 
	D3D12_RECT m_Scissor;

	bool CreateRenderTarget();
	bool CreateDepthStencil();

	UINT m_RtvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap = nullptr;
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT] = { nullptr };

	UINT m_DsvDescriptorSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap = nullptr;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr;

	ID3D12Resource* m_currentRenderTarget = nullptr;
	void WaitRender();
};
