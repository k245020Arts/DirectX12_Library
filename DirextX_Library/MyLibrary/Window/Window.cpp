#include "Window.h"
#include <assert.h>
#include "../../ImGui/imgui.h"
//マウス用
#include <Windows.h>
#include <Windowsx.h>

//using namespace DirectX;

//HRESULT D3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MiniumuFeatureLevel, REFIID riid, void** ppDevice);

#include "../Input/InputManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	//ImGuiがキーボード入力を占有している場合は、ゲーム側の処理をスキップ
	if (ImGui::GetCurrentContext() != nullptr){
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard) {
			// ただし、キーが離されたイベント(WM_KEYUP)だけは処理しておかないと、
			// ImGuiからゲームに戻った時にキーが「押しっぱなし」と誤認されるのを防ぐため、最低限のケアをします。
			if (msg == WM_KEYUP) {
				InputManager::GetInstance()->SetCurrentKeyCode(static_cast<int>(wparam), false);
			}
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}

	switch (msg)
	{
		//キーが押されたとき
	case WM_KEYDOWN:
		InputManager::GetInstance()->SetCurrentKeyCode(static_cast<int>(wparam), true);
		break;

		//キーが離されたとき
	case WM_KEYUP:
		InputManager::GetInstance()->SetCurrentKeyCode(static_cast<int>(wparam), false);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MOUSEMOVE:

		InputManager::GetInstance()->SetMousePosition(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		return 0;

	case WM_LBUTTONDOWN:
		InputManager::GetInstance()->SetMouseButtonState(0, true);
		return 0;

	case WM_LBUTTONUP:
		InputManager::GetInstance()->SetMouseButtonState(0, false);
		return 0;

	case WM_RBUTTONDOWN:
		InputManager::GetInstance()->SetMouseButtonState(1, true);
		return 0;

	case WM_RBUTTONUP:
		InputManager::GetInstance()->SetMouseButtonState(1, false);
		return 0;

	case WM_MBUTTONDOWN:
		InputManager::GetInstance()->SetMouseButtonState(2, true);
		return 0;

	case WM_MBUTTONUP:
		InputManager::GetInstance()->SetMouseButtonState(2, false);
		return 0;

	case WM_MOUSEWHEEL:
		InputManager::GetInstance()->SetWheelDelta(GET_WHEEL_DELTA_WPARAM(wparam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
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
