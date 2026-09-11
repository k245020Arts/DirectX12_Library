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

int AlignmentedSize(size_t size, size_t alignment) 
{
	return size + alignment - size % alignment;
}


Window::~Window()
{
	_backBuffers.clear();
}

bool Window::Create(int _cWidth, int _cHeight, const std::wstring& _titleName, const std::wstring& _windowClassName)
{
	angle = 0.0f;
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

	//PMDヘッダ構造体
	struct PMDHeader {
		float version; //例：00 00 80 3F == 1.00
		char model_name[20];//モデル名
		char comment[256];//モデルコメント
	};
	char pmdsignature[3];
	PMDHeader pmdheader = {};
	FILE* fp;
	auto err = fopen_s(&fp, "data/Model/初音ミク.pmd", "rb");
	if (fp == nullptr) {
		assert(false && "ファイルが開けませんでした");
		return false;
	}
	fread(pmdsignature, sizeof(pmdsignature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	constexpr size_t pmdVertex_size = 38; //頂点一つあたりのサイズ

	
	fread(&vertNum, sizeof(vertNum), 1, fp);

	std::vector<unsigned char> pmdvertices(vertNum * pmdVertex_size); //バッファーの確保
	fread(pmdvertices.data(), pmdvertices.size(), 1, fp);

	/*std::vector<PMDVertex> gpuVertices(vertNum);

	for (unsigned int i = 0; i < vertNum; ++i)
	{
		const unsigned char* src = pmdvertices.data() + i * pmdVertex_size;

		memcpy(&gpuVertices[i].pos,src + 0,sizeof(DirectX::XMFLOAT3));

		memcpy(&gpuVertices[i].normal,src + 12,sizeof(DirectX::XMFLOAT3));

		memcpy(&gpuVertices[i].uv,src + 24,sizeof(DirectX::XMFLOAT2));

		memcpy(&gpuVertices[i].boneNo,src + 32,sizeof(unsigned short) * 2);

		memcpy(&gpuVertices[i].boneWeight,src + 36,sizeof(unsigned char));

		memcpy(&gpuVertices[i].edgeFlg,src + 37,sizeof(unsigned char));

		gpuVertices[i].dummy = 0;
	}*/

	fread(&indicsNum, sizeof(indicsNum), 1, fp);

	//インデックスデータの作成
	std::vector<unsigned short> indices;
	indices.resize(indicsNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	fclose(fp);

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

	//SRGB用のレンダーターゲットビュー設定を行う
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //ガンマ補正あり
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (unsigned int idx = 0; idx < swcDesc.BufferCount; ++idx) {
		result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));

		//ディスクプリタヒープの先頭のアドレス(ハンドル)を取得し、それを先ほど取得したバッファを使用しターゲットビューを生成する

		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

		//GetCPUDescriptorHandleForHeapStart()のメソッドでとれるのは先頭アドレスなので1番目以降の
		//ディスクリプタを取得するためには一つ分ずらしてあげなければいけないので下の処理を行う
		//通常のポインタみたいに++は出来ない

		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		_dev->CreateRenderTargetView(_backBuffers[idx].Get(), &rtvDesc, handle); //第二引数をnullptrにするとフォーマットをスワップチェーンに準拠するということを意味する


	}
	//フェンスを作成する
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);

	UpdateWindow(hwnd);

	adapters.clear();//念のため削除

	//Vertex vertices[] = {
	//	{{ 0.0f,100.0f,0.0f},{0.0f,1.0f}} ,//左下
	//	{{ 0.0f,0.0f,0.0f},{0.0f,0.0f}} ,//左上
	//	{{100.0f,100.0f,0.0f},{1.0f,1.0f}} ,//右下
	//	{{100.0f,0.0f,0.0f},{1.0f,0.0f}} ,//右上
	//};

	Vertex vertices[] = {
		{{-1.0f,-1.0f,0.0f},{0.0f,1.0f} },//左下
		{{-1.0f,1.0f,0.0f} ,{0.0f,0.0f}},//左上
		{{1.0f,-1.0f,0.0f} ,{1.0f,1.0f}},//右下
		{{1.0f,1.0f,0.0f} ,{1.0f,0.0f}},//右上
	};

	//頂点バッファの設定

	auto heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resdesc = CD3DX12_RESOURCE_DESC::Buffer(pmdvertices.size() *  pmdVertex_size);

	//頂点バッファの生成
	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

	//頂点情報のコピー
	unsigned char* vertMap = nullptr;
	//バッファの仮想アドレスを取得する関数、CPUで変更をすればGPUで変更が出来るように出来る
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	std::copy(pmdvertices.begin(),pmdvertices.end(),vertMap);

	vertBuff->Unmap(0, nullptr);

	//頂点バッファービューの作成

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファーの仮想アドレス
	vbView.SizeInBytes = (UINT)pmdvertices.size();//全体ののバイト数
	vbView.StrideInBytes = (UINT)pmdVertex_size;//1頂点あたりのバイト数

	//バッファーのサイズ以外は頂点シェーダーの設定を使いまわしても良い
	heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resdesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));
	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));

	unsigned short* idxMap = nullptr;
	//バッファの仮想アドレスを取得する関数、CPUで変更をすればGPUで変更が出来るように出来る
	result = idxBuff->Map(0, nullptr, (void**)&idxMap);

	std::copy(std::begin(indices), std::end(indices), idxMap);

	idxBuff->Unmap(0, nullptr);

	//インデックスバッファービューの作成
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.SizeInBytes = indices.size() *  sizeof(indices[0]);
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
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		//{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	//深度バッファの作成
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; //2Dテクスチャ用
	depthResDesc.Width = (UINT64)_cWidth; //レンダーターゲットと同じ値
	depthResDesc.Height = (UINT64)_cHeight;
	depthResDesc.DepthOrArraySize = 1; //テクスチャ配列でも、3Dテクスチャでもない
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT; //深度書き込み用フォーマット
	depthResDesc.SampleDesc.Count = 1; //サンプルは一ピクセルあたり一つ

	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; //デプスステンシルとして活用

	//深度地用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトなので後はunknownで良い
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//クリアバリューの生成(大事)
	D3D12_CLEAR_VALUE depthClearValue = {};

	depthClearValue.DepthStencil.Depth = 1.0f; //深さ1.0f (最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32ビットfloatとして定義

	result = _dev->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; //デプスステンシルビューとして扱う
	dsvHeapDesc.NumDescriptors = 1; //深度ビューは一つ

	result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE; //フラグなし

	_dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr; //後で設定する

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.StencilEnable = false;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さい方を採用;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

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

	//gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;;

	//レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //0～1に正規化させたRGBA

	//アンチエイリアスの設定
	gpipeline.SampleDesc.Count = 1; //サンプリングは1ピクセルにつき1
	gpipeline.SampleDesc.Quality = 0; //クオリティーは最低

	//ポリゴン生成時のルートシグネチャの管理

	// 
	//ルートシグネチャの生成
	//D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	//rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;//頂点情報があることを示す

	//ID3DBlob* rootSigBlob = nullptr;

	////バイナリコードの生成
	//result = D3D12SerializeRootSignature(
	//	&rootSignatureDesc,
	//	D3D_ROOT_SIGNATURE_VERSION_1_0, //ルートシグネチャのバージョン
	//	&rootSigBlob, //シェーダーを生成した時と同じ感じ
	//	&errorBlob
	//);

	//result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	//rootSigBlob->Release(); ComPtrを使っているので手動でReleaseはしない、通常のポインタの場合は絶対に忘れてはいけない

	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage scratchImg = {};

	result = DirectX::LoadFromWICFile(L"data/textest.png", DirectX::WIC_FLAGS_NONE, &metadata, scratchImg);

	auto image = scratchImg.GetImage(0, 0, 0);

	//自前で作成する場合はこれを使う

	/*struct TexRGBA
	{
		unsigned char R, G, B, A;
	};

	std::vector<TexRGBA> texturedata(256 * 256);

	for (auto& rgba : texturedata) {
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;

	}*/

	//テクステャバッファーの作成

	//WriteToSubresouceで転送するためのヒープ設定
	D3D12_HEAP_PROPERTIES upLoadheapProp = {};

	//UpLoadにする
	upLoadheapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	//アップロード用に使用する前提なのでUNKNOWNで良い
	upLoadheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	upLoadheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//単一アダプタのため0
	upLoadheapProp.CreationNodeMask = 0;
	upLoadheapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};

	resDesc.Format = DXGI_FORMAT_UNKNOWN; //単なるデータの固まりなのでUNKONWN
	resDesc.Width = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * image->height; //データサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1; //通常テクステャなのでアンチエイリシアリングしない
	resDesc.SampleDesc.Quality = 0; //最低クオリティ
	resDesc.MipLevels = 1; //ミップアップしないのでミップ数は1つ : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //単なるバッファとして生成
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //連続したレイアウト
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //フラグなし

	_dev->CreateCommittedResource(&upLoadheapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuff));

	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT; //テクスチャ用

	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//単一アダプタのため0
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	resDesc.Format = metadata.format; //RGBAフォーマットDXGI_FORMAT_R8G8B8A8_UNORM : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Width = static_cast<UINT>(metadata.width); //幅 255 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Height = (UINT)metadata.height; //高さ 255 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.DepthOrArraySize = (UINT16)metadata.arraySize; //2Dで配列でもないので1 : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.SampleDesc.Count = 1; //通常テクステャなのでアンチエイリシアリングしない
	resDesc.SampleDesc.Quality = 0; //最低クオリティ
	resDesc.MipLevels = (UINT16)metadata.mipLevels; //ミップアップしないのでミップ数は1つ : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension); //２Dテクステャ用 D3D12_RESOURCE_DIMENSION_TEXTURE2D : DirectXTexを使用すると画像のデータが取得できるのでそれを使う
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; //レイアウトは設定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //フラグなし

	result = _dev->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texbuff));
	//GPUにデータ転送
	//result = texbuff->WriteToSubresource(0, nullptr, texturedata.data(), sizeof(TexRGBA) * 256, sizeof(TexRGBA) * texturedata.size());
	//result = texbuff->WriteToSubresource(0, nullptr, image->pixels, (UINT)image->rowPitch, (UINT)image->slicePitch);

	uint8_t* mapforImg = nullptr;//image->pixelsと同じ型にする
	result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);//マップ
	auto srcAddress = image->pixels;
	auto rowPitch = AlignmentedSize(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < image->height; ++y) {

		std::copy_n(srcAddress, image->rowPitch,mapforImg);//コピー
		//1行ごとの辻褄を合わせてやる
		srcAddress += image->rowPitch;
		mapforImg += rowPitch;
	}

	D3D12_TEXTURE_COPY_LOCATION src = {};

	//コピー元(アップロード側)の設定
	src.pResource = uploadBuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; //フットプリント指定
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = metadata.width;
	src.PlacedFootprint.Footprint.Height = metadata.height;
	src.PlacedFootprint.Footprint.Depth = metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(image->rowPitch,D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = image->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	//コピー先指定
	dst.pResource = texbuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX; //インデックスの指定
	dst.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = texbuff.Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();
	//コマンドリストの実行
	ComPtr<ID3D12CommandList> cmdlists[] = { _cmdList.Get()};
	_cmdQueue->ExecuteCommandLists(1, cmdlists->GetAddressOf());
	////待ち
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	_cmdAllocator->Reset();//キューをクリア
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

	//シェーダーリソースビューの作成
	//ディスプリタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	descHeapDesc.NodeMask = 0;

	descHeapDesc.NumDescriptors = 2;

	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; //シェーダーリソースビュー用

	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	//シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = metadata.format; //0.0f～1.0fに初期化 DXGI_FORMAT_R8G8B8A8_UNORM
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクステャ用

	srvDesc.Texture2D.MipLevels = 1; //ミップマップを使用しないので1

	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateShaderResourceView(texbuff.Get(), &srvDesc,basicHeapHandle );

	DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
	worldMatrix = DirectX::XMMatrixIdentity();

	DirectX::XMFLOAT3 eye(0, 10, -15);
	DirectX::XMFLOAT3 target(0, 10, 0);
	DirectX::XMFLOAT3 up(0, 1, 0);

	viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));

	projectionMatrix =  DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, static_cast<float>(_cWidth) / static_cast<float>(_cHeight), 1.0f, 100.0f);

	matrix = worldMatrix;
	matrix *= viewMatrix;
	matrix *= projectionMatrix;

	/*matrix.r[0].m128_f32[0] = 2.0f / _cWidth;
	matrix.r[1].m128_f32[1] = -2.0f / _cHeight;

	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = 1.0f;*/

	CD3DX12_HEAP_PROPERTIES constheapProps(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC constresourceDesc =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatriceData) + 0xff) & ~0xff);

	result = _dev->CreateCommittedResource(&constheapProps, D3D12_HEAP_FLAG_NONE, &constresourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuff));

	
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix); //マップ

	//ワールドの行列を入れてからビュープロジェクションの行列を入れることによって、順番に入るようにしている
	mapMatrix->world = worldMatrix;
	mapMatrix->viewproj = viewMatrix * projectionMatrix;


	basicHeapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constBuff->GetDesc().Width;

	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	//ディスクプリタレンジの設定
	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};

	//テクステャ用レジスター0番
	descTblRange[0].NumDescriptors = 1; //複数のテクスチャがディスクプリタヒープ上で並んでおり、連続で指定する場合はこの数が増える
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //種別はテクステャ
	descTblRange[0].BaseShaderRegister = 0; //0番スロットから
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[1].NumDescriptors = 1; //定数一つ
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; //種別は定数
	descTblRange[1].BaseShaderRegister = 0; //0番スロットから
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	//ルートパラメーターの設定
	D3D12_ROOT_PARAMETER rootParam[2] = {};

	//テクスチャバッファの生成
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //ピクセルシェーダーから見える
	rootParam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //頂点シェーダーから見える
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;

	//ルートパラメーターの設定 all指定版
	//D3D12_ROOT_PARAMETER rootParam = {};

	//rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全てのシェーダーから見える
	//rootParam.DescriptorTable.pDescriptorRanges = descTblRange;
	//rootParam.DescriptorTable.NumDescriptorRanges = 2;

	//ディスクプリタテーブルの作成
	//画像用のルートシグネチャ
	D3D12_ROOT_SIGNATURE_DESC rootsignatureDesc = {};

	rootsignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //頂点情報があるかを示す

	rootsignatureDesc.pParameters = rootParam;
	rootsignatureDesc.NumParameters = 2;

	//サンプラーの生成
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //横方向の繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //縦方向の繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //奥行きの繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //ボーダーは黒

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //線形補完
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; //ミップマップ最大値
	samplerDesc.MinLOD = 0.0f; //ミップマップ最低値
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //ピクセルシェーダーから見える
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //リサンプリングしない

	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;

	rootsignatureDesc.pStaticSamplers = &samplerDesc;
	rootsignatureDesc.NumStaticSamplers = 1;


	result = D3D12SerializeRootSignature(&rootsignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));

	gpipeline.pRootSignature = rootsignature.Get();

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

void Window::Update()
{
	angle += 0.01f;
	worldMatrix = DirectX::XMMatrixRotationY(angle);
	mapMatrix->world = worldMatrix;
	mapMatrix->viewproj = viewMatrix * projectionMatrix;
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
	auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	plus += 0.01f;
	float r = sinf(plus);

	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);
	_cmdList->SetGraphicsRootSignature(rootsignature.Get());

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //トライアングルリストの生成
	//_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); //点を描画
	//D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST : 三角形を描画するときに使う
	//D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : 四角形を描画す際に使う
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	_cmdList->IASetIndexBuffer(&ibView);
	_cmdList->SetGraphicsRootSignature(rootsignature.Get());
	_cmdList->SetDescriptorHeaps(1, basicDescHeap.GetAddressOf());

	//--------ルートパラメーターを個別で指定するならこれらをひとまとめとして書く必要がある-------

	auto heapHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	_cmdList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

	heapHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	_cmdList->SetGraphicsRootDescriptorTable(1, heapHandle);

	//-------------------------------------------------------------------------------------------

	//第一引数に頂点数を代入
	//_cmdList->DrawInstanced(vertNum, 1, 0, 0);

	//第一引数にインデックスの数を代入
	_cmdList->DrawIndexedInstanced(indicsNum, 1, 0, 0,0);

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
