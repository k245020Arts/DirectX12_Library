#include "Engine.h"

Engine::~Engine()
{
    if (commandQueue && fence)
    {
        WaitRender();
    }

    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

bool Engine::Init(HWND _hwnd, const Size& _size)
{
    hwnd = _hwnd;

    windowSize = _size;

    bool deviceCreate = CreateDevice();

    if (!deviceCreate) {
        OutputDebugStringW(L"デバイスの出力に失敗しました");
        return false;
    }

    CreateAdapter();

    bool commandQueueCreate = CreateCommandQueue();
    if (!commandQueueCreate)
    {
        OutputDebugStringW(L"コマンドキューの生成に失敗");
        return false;
    }

    bool swapChainCreate = CreateSwapChain();
    if (!swapChainCreate)
    {
        OutputDebugStringW(L"スワップチェインの生成に失敗");
        return false;
    }

    bool commandListCreate = CreateCommandList();
    if (!commandListCreate)
    {
        OutputDebugStringW(L"コマンドリストの生成に失敗");
        return false;
    }
    
    bool fenceCreate = CreateFence();
    if (!fenceCreate)
    {
        OutputDebugStringW(L"フェンスの生成に失敗");
        return false;
    }

    CreateViewPort();
    CreateScissorRect();

    bool renderTargetCreate = CreateRenderTarget();

    if (!renderTargetCreate)
    {
        OutputDebugStringW(L"レンダーターゲットの生成に失敗");
        return false;
    }

    bool depthBuffer =  CreateDepthBuffer();
    if (!depthBuffer)
    {
        OutputDebugStringW(L"デプスステンシルバッファの生成に失敗\n");
        return false;
    }

    return true;
}

void Engine::BeginRender()
{
    // 現在のレンダーターゲットを更新
    currentRenderTarget = pRenderTargets[currentBackBufferIndex].Get();

    // コマンドを初期化してためる準備をする
    commandAllocator[currentBackBufferIndex]->Reset();
    commandList->Reset(commandAllocator[currentBackBufferIndex].Get(), nullptr);

    // ビューポートとシザー矩形を設定
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    // 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
    auto currentRtvHandle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    currentRtvHandle.ptr += currentBackBufferIndex * m_RtvDescriptorSize;

    // 深度ステンシルのディスクリプタヒープの開始アドレス取得
    auto currentDsvHandle = pDsvHeap->GetCPUDescriptorHandleForHeapStart();

    // レンダーターゲットが使用可能になるまで待つ
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentRenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    // レンダーターゲットを設定
    commandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

    // レンダーターゲットをクリア
    const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    commandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

    // 深度ステンシルビューをクリア
    commandList->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}


void Engine::EndRender()
{
    // レンダーターゲットに書き込み終わるまで待つ
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    // コマンドの記録を終了
    commandList->Close();

    // コマンドを実行
    ID3D12CommandList* ppCmdLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCmdLists);

    // スワップチェーンを切り替え
    swapChain->Present(1, 0);

    // 描画完了を待つ
    WaitRender();

    // バックバッファ番号更新
    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

ID3D12Device6* Engine::Device()
{
    return pDevice.Get();
}

ID3D12GraphicsCommandList* Engine::CommandList()
{
    return commandList.Get();
}

UINT Engine::CurrentBackBufferIndex()
{
    return currentBackBufferIndex;
}

void Engine::CreateDebugLayer()
{

#ifdef _DEBUG

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }

#endif // _DEBUG

}

bool Engine::CreateDevice()
{

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
        if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&pDevice)) == S_OK) {
            featureLevel = lv;
            return true;
        }
    }

    return false;
}

bool Engine::CreateAdapter()
{

#ifdef _DEBUG
    auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
#else
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
#endif // _DEBUG

    if (result != S_OK) {
        assert(false && "アダプターの取得ミス");
        return false;
    }

    std::vector<ComPtr<IDXGIAdapter>> adapters;

    ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

    for (int i = 0; dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
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

    return true;
}

bool Engine::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT; //タイムアウトなし

    cmdQueueDesc.NodeMask = 0; //アダプターを一つしか使用しないときは0

    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; //プライオリティーに特に変更なし

    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT hr = pDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue));

    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool Engine::CreateSwapChain()
{
    //スワップチェーン
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

    swapChainDesc.Width = windowSize.width; //画面の幅
    swapChainDesc.Height = windowSize.height; //画面の高さ
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

    auto hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, swapChain1.GetAddressOf());

    if (FAILED(hr))
    {
        return false;
    }

    hr = swapChain1.As(&swapChain);

    if (FAILED(hr))
    {
        return false;
    }

    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

    return true;
}

bool Engine::CreateCommandList()
{

    // コマンドアロケーターの作成
    HRESULT hr;
    commandAllocator.resize(FRAME_BUFFER_COUNT);
    for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator[i].ReleaseAndGetAddressOf()));
    }

    if (FAILED(hr))
    {
        return false;
    }

    // コマンドリストの生成
    hr = pDevice->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,commandAllocator[currentBackBufferIndex].Get(), nullptr,IID_PPV_ARGS(&commandList));

    if (FAILED(hr))
    {
        return false;
    }

    //コマンドリストは開かれている状態で作成されるので、いったん閉じる。
    commandList->Close();
    return true;
}

bool Engine::CreateFence()
{
    auto hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        return false;
    }

    m_fenceValue = 0;

    m_fenceEvent = CreateEvent( nullptr,FALSE,FALSE, nullptr );

    if (m_fenceEvent == nullptr)
    {
        return false;
    }

    return true;
}

void Engine::CreateViewPort()
{
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(windowSize.width);
    viewport.Height = static_cast<float>(windowSize.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

void Engine::CreateScissorRect()
{
    scissor.left = 0;
    scissor.right = windowSize.width;
    scissor.top = 0;
    scissor.bottom = windowSize.height;
}

bool Engine::CreateRenderTarget()
{
    //レンダーターゲットビューの設定
    //ディスクリプタヒープの作成
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビューなのでRTV

    heapDesc.NodeMask = 0; //GPUが2つ以上ある場合の識別に使われる。一つなら0で良し
    heapDesc.NumDescriptors = 2; //表裏の2つ
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//指定なし


    auto hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pRtvHeap));

    if (FAILED(hr)) {
        return false;
    }

    //スワップチェーン上のバックバッファーとディスクプリタの紐づけを行う
    DXGI_SWAP_CHAIN_DESC swcDesc = {};

    hr = swapChain->GetDesc(&swcDesc);

    pRenderTargets.resize(swcDesc.BufferCount);

    //SRGB用のレンダーターゲットビュー設定を行う
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //ガンマ補正あり
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (unsigned int idx = 0; idx < swcDesc.BufferCount; ++idx) {
        hr = swapChain->GetBuffer(idx, IID_PPV_ARGS(&pRenderTargets[idx]));

        if (FAILED(hr)) {
            return false;
        }

        //ディスクプリタヒープの先頭のアドレス(ハンドル)を取得し、それを先ほど取得したバッファを使用しターゲットビューを生成する

        D3D12_CPU_DESCRIPTOR_HANDLE handle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();

        //GetCPUDescriptorHandleForHeapStart()のメソッドでとれるのは先頭アドレスなので1番目以降の
        //ディスクリプタを取得するためには一つ分ずらしてあげなければいけないので下の処理を行う
        //通常のポインタみたいに++は出来ない

        handle.ptr += idx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        pDevice->CreateRenderTargetView(pRenderTargets[idx].Get(), &rtvDesc, handle); //第二引数をnullptrにするとフォーマットをスワップチェーンに準拠するということを意味する


    }

    return true;
}

bool Engine::CreateDepthBuffer()
{

    //深度バッファの作成
    D3D12_RESOURCE_DESC depthResDesc = {};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; //2Dテクスチャ用
    depthResDesc.Width = (UINT64)windowSize.width; //レンダーターゲットと同じ値
    depthResDesc.Height = (UINT64)windowSize.height;
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

    auto hr = pDevice->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&pDepthStencilBuffer));

    if (FAILED(hr)) {
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; //デプスステンシルビューとして扱う
    dsvHeapDesc.NumDescriptors = 1; //深度ビューは一つ

    hr = pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&pDsvHeap));

    if (FAILED(hr)) {
        return false;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE; //フラグなし

    pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), &dsvDesc, pDsvHeap->GetCPUDescriptorHandleForHeapStart());

    return true;
}

void Engine::WaitRender()
{
    //描画終了待ち
    const UINT64 fenceValue = ++m_fenceValue;

    HRESULT hr = commandQueue->Signal(fence.Get(),fenceValue);

    // 次のフレームの描画準備がまだであれば待機する.
    if (fence->GetCompletedValue() < fenceValue)
    {
        // 完了時にイベントを設定.
        auto hr = fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        if (FAILED(hr))
        {
            return;
        }

        // 待機処理.
        if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE))
        {
            return;
        }
    }
}
