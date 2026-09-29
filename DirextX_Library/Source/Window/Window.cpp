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
	
}

bool Window::Create(const Size& _size, const std::wstring& _titleName, const std::wstring& _windowClassName) 
{

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	w.lpszClassName = _windowClassName.c_str(); //アプリケーションクラス名
	w.hInstance = GetModuleHandle(0); //ハンドルの取得

	if (!RegisterClassEx(&w)) { //アプリケーションクラス(ウィンドウクラスの指定をOSに伝える)
		return false;
	}

	RECT wrc = { 0,0,_size.width,_size.height };

	windowSize.width = _size.width;
	windowSize.height = _size.height;

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(
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


	// ウィンドウを表示
	ShowWindow(hwnd, SW_SHOWNORMAL);

	// ウィンドウにフォーカスする
	SetFocus(hwnd);


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
