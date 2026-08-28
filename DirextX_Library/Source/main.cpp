#include <string>
#include "Window/Window.h"
#include <assert.h>
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG


#include <vector>

void DebugOutputFormatString(const char* format, ...) 
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif // _DEBUG

}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	
	Window window;
	if (!window.Create(1200, 500, L"DX12_Library", L"Window")) { //‰Šú‰»
		assert(false && "ƒEƒBƒ“ƒhƒEì¬¸”s");
		return 0;
	}
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}
		bool result = window.ScreenFlip();
		if (!result) {
			assert(false && "”½“]‚É¸”s‚µ‚Ü‚µ‚½");
			break;
		}
	}
	return 1;
}

