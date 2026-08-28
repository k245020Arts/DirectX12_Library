#include "Window.h"
#include <assert.h>

//using namespace DirectX;

//HRESULT D3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MiniumuFeatureLevel, REFIID riid, void** ppDevice);

LRESULT WindowProcedure(HWND hwud, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY) {
		PostQuitMessage(0); // OSに対してこのアプリは終わると伝える
		return 0;
	}
	return DefWindowProc(hwud, msg, wparam, lparam);
}


Window::~Window()
{
	_backBuffers.clear();
}

bool Window::Create(int _cWidth, int _cHeight, const std::wstring& _titleName, const std::wstring& _windowClassName)
{

	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _windowClassName.c_str(); //アプリケーションクラス名
	w.hInstance = GetModuleHandle(0); //ハンドルの取得

	if (!RegisterClassEx(&w)) { //アプリケーションクラス(ウィンドウクラスの指定をOSに伝える)
		return false;
	}

	RECT wrc = { 0,0,_cWidth,_cHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		w.lpszClassName, //クラス名指定
		_titleName.c_str(),//タイトルバーの文字(ウィンドウの左上に表示される名前)
		WS_OVERLAPPEDWINDOW, //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT, //表示X座標はOSにお任せ
		CW_USEDEFAULT, //表示Y座標はOSにお任せ
		wrc.right - wrc.left, //ウィンドウ幅
		wrc.bottom - wrc.top, //ウィンドウ高
		nullptr, //親ウィンドウハンドル
		nullptr, //メニューハンドル
		w.hInstance, //呼び出しアプリケーションハンドル
		nullptr); //追加パラメーター

#ifdef _DEBUG
	
	ComPtr<ID3D12Debug> debugController;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

	

#endif // _DEBUG


	D3D_FEATURE_LEVEL featureLevel;

	

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,

	};

	//適切なフィーチャーレベルの選択
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = lv;
			break;
		}
	}
#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG

	

	if (result != S_OK) {
		assert(false && "アダプターの取得ミス");
		return 0;
	}

	std::vector<ComPtr<IDXGIAdapter>> adapters;

	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.emplace_back(tmpAdapter);
	}

	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); //アダプターの説明オブジェクトの取得

		std::wstring strDesc = adesc.Description;

		//探したいアダプターの名前を確認
		if (strDesc.find(L"NVDIA") != std::string::npos) {
			tmpAdapter = adpt; //TODOまだ入らない
			break;
		}
	}

	//コマンドアロケーターの設定
	auto resultAllocator = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));

	if (_cmdAllocator == nullptr) { //nullチェックこれを省くと下でwaringが出る
		assert(false && "_cmdAllocator is null. Command Allocator creation failed.");
		return 0;
	}
	//コマンドリストの設定
	auto resultList = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList));
	if (FAILED(resultList)) {
		assert(false && "Failed to create command list.");
		return 0;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT; //タイムアウトなし

	cmdQueueDesc.NodeMask = 0; //アダプターを一つしか使用しないときは0

	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //プライオリティーに特に変更なし

	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	if (_cmdQueue == nullptr) {
		assert(false && "キューコマンドがヌル");
		return false;
	}

	//スワップチェーン
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = _cWidth; //画面の幅
	swapChainDesc.Height = _cHeight; //画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //ピクセルフォーマット
	swapChainDesc.Stereo = false; //ステレオ表示フラグ
	swapChainDesc.SampleDesc.Count = 1; //マルチサンプルの指定
	swapChainDesc.SampleDesc.Quality = 0; //マルチサンプルの指定
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2; //ダブルバッファーなら2で良い

	//バックバッファーは伸び縮み完了
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

	//フリップ後は素早く破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//特に指定なし
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//ウィンドウフルスクリーン切り替え可能
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapChain1;

	result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, swapChain1.GetAddressOf());

	if (SUCCEEDED(result))
	{
		result = swapChain1.As(&_swapChain);
	}

	//レンダーターゲットビューの設定
	//ディスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビューなのでRTV

	heapDesc.NodeMask = 0; //GPUが2つ以上ある場合の識別に使われる。一つなら0で良し
	heapDesc.NumDescriptors = 2; //表裏の2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//指定なし

	
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//スワップチェーン上のバックバッファーとディスクプリタの紐づけを行う
	DXGI_SWAP_CHAIN_DESC swcDesc = {};

	result = _swapChain->GetDesc(&swcDesc);

	_backBuffers.resize(swcDesc.BufferCount);
	for (unsigned int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));

		//ディスクプリタヒープの先頭のアドレス(ハンドル)を取得し、それを先ほど取得したバッファを使用しターゲットビューを生成する

		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		//GetCPUDescriptorHandleForHeapStart()のメソッドでとれるのは先頭アドレスなので1番目以降の
		//ディスクリプタを取得するためには一つ分ずらしてあげなければいけないので下の処理を行う
		//通常のポインタみたいに++は出来ない

		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[idx].Get(), nullptr, handle);


	}
	//フェンスを作成する
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);

	UpdateWindow(hwnd);

	adapters.clear();//念のため削除

	DirectX::XMFLOAT3 vertices[] = {
		{-0.4f,-0.7f,0.0f} ,//左下
		{-0.4f,0.7f,0.0f} ,//左上
		{0.4f,-0.7f,0.0f} ,//右下
		{0.4f,0.7f,0.0f} ,//右上
	};

	//頂点バッファの設定

	//頂点ヒープの設定
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;

	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソース設定構造体
	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;

	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点バッファの生成
	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

	//頂点情報のコピー
	DirectX::XMFLOAT3* vertMap = nullptr;
	//バッファの仮想アドレスを取得する関数、CPUで変更をすればGPUで変更が出来るように出来る
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	vertBuff->Unmap(0, nullptr);

	//頂点バッファービューの作成

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファーの仮想アドレス
	vbView.SizeInBytes = sizeof(vertices);//全体ののバイト数
	vbView.StrideInBytes = sizeof(vertices[0]);//1頂点あたりのバイト数


	//インデックスデータの作成
	unsigned short indices[] = {
		0,1,2,
		2,1,3
	};

	//バッファーのサイズ以外は頂点シェーダーの設定を使いまわしても良い
	resdesc.Width = sizeof(indices);

	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));

	unsigned short* idxMap = nullptr;
	//バッファの仮想アドレスを取得する関数、CPUで変更をすればGPUで変更が出来るように出来る
	result = idxBuff->Map(0, nullptr, (void**)&idxMap);

	std::copy(std::begin(indices), std::end(indices), idxMap);

	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューの作成
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(indices);
	ibView.Format = DXGI_FORMAT_R16_UINT;

	//シェーダーオブジェクトの生成
	ComPtr<ID3D10Blob> vsBlob = nullptr;//頂点シェーダー用
	ComPtr<ID3D10Blob> psBlob = nullptr;//ピクセルシェーダー用
	ComPtr<ID3D10Blob> errorBlob = nullptr;

	result = D3DCompileFromFile(L"Shader/Basic_VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	result = D3DCompileFromFile(L"Shader/Basic_PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr; //後で設定する

	//頂点シェーダーの設定
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	//ピクセルシェーダーの設定
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//まだアンチエイリアスは使わないためfalse
	gpipeline.RasterizerState.MultisampleEnable = false;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpipeline.RasterizerState.DepthClipEnable = true;

	//alphaテストを行わないならfalse
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	renderTargetBlendDesc.BlendEnable = false; //ブレンドをするならtrue
	//renderTargetBlendDesc.BlendEnable = true;

	//----------------------------------αブレンド----------------------------------------
	//renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	//renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	////Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	//renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	//renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	//renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	//-------------------------------------------------------------------------------------

	//----------------------------------加算ブレンド----------------------------------------
	//renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	//renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	//renderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;

	////Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	//renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	//renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	//renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	//-------------------------------------------------------------------------------------

	//----------------------------------乗算ブレンド----------------------------------------
	/*renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_SRC_COLOR;

	Alphag側も設定しないと正常にパイプラインがCreate出来ないため一応書く
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;*/

	//-------------------------------------------------------------------------------------

	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//要素数の計算

	//切り離せない頂点集合を特定のインデックスで切り離すための指定を行うためのものだが、切り離さないので以下の指定をする
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	// ★重要
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0～1に正規化させたRGBA

	//アンチエイリアスの設定
	gpipeline.SampleDesc.Count = 1; //サンプリングは1ピクセルにつき1
	gpipeline.SampleDesc.Quality = 0; //クオリティーは最低

	//ルートシグネチャの生成
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;//頂点情報があることを示す

	ID3DBlob* rootSigBlob = nullptr;

	//バイナリコードの生成
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0, //ルートシグネチャのバージョン
		&rootSigBlob, //シェーダーを生成した時と同じ感じ
		&errorBlob
		);

	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	//rootSigBlob->Release(); ComPtrを使っているので手動でReleaseはしない、通常のポインタの場合は絶対に忘れてはいけない


	gpipeline.pRootSignature = rootSignature.Get();

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineState));

	//ビューポートの設定 : ビューポートとは、ウィンドウに対してレンダリング結果をどう表示するかの設定

	viewport.Width = (FLOAT)_cWidth;
	viewport.Height = (FLOAT)_cHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//シザー短形の設定 : シザー短形とは、ビューポートで出力された画像のどこからどこまでを実際に画像に映し出すかを設定する

	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = _cWidth;
	scissorrect.bottom = _cHeight;




	return true;
}

bool Window::ProcessMessage()
{
	MSG msg = {};
	if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		return false;
	}
	

	return true;
}

bool Window::ScreenFlip()
{
	//バックバッファーを指すIndexを取得する
	auto bbIdx = _swapChain->GetCurrentBackBufferIndex();

	//バリアを作成する
	D3D12_RESOURCE_BARRIER  BarrierDesc = {};

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//遷移
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;//特に指定なし
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx].Get();//バックバッファーリソース
	BarrierDesc.Transition.Subresource = 0;

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	_cmdList->ResourceBarrier(1, &BarrierDesc);

	_cmdList->SetPipelineState(pipelineState.Get());

	//これから使用されるであろうバックバッファーをレンダーターゲットビューとしてセットする
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	plus += 0.01f;
	float r = sinf(plus);

	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };//白色

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	
	_cmdList->SetGraphicsRootSignature(rootSignature.Get());
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //トライアングルリストの生成
	//D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST : 三角形を描画するときに使う
	//D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : 四角形を描画す際に使う
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	_cmdList->IASetIndexBuffer(&ibView);
	//第一引数に頂点数を代入
	//_cmdList->DrawInstanced(3, 1, 0, 0);

	//第一引数にインデックスの数を代入
	_cmdList->DrawIndexedInstanced(6, 1, 0, 0,0);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	//命令のクローズを忘れずに
	_cmdList->Close();

	ComPtr<ID3D12CommandList> cmdlists[] = { _cmdList.Get() };

	_cmdQueue->ExecuteCommandLists(1, cmdlists->GetAddressOf());

	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);

		if (event == nullptr)
		{
			assert(false && "CreateEvent失敗");
			return false;
		}

		_fence->SetEventOnCompletion(_fenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	_cmdAllocator->Reset();//キューをクリア
	_cmdList->Reset(_cmdAllocator.Get(), pipelineState.Get());//再びコマンドリストをためる準備

	_swapChain->Present(1, 0);//第一引数は待つべき垂直同期の数

	return true;
}
