#include <string>
#include "Window/Window.h"
#include <string>
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
	if (!window.Create(1200, 500, L"DX12_Library", L"Window")) {
		assert(false && "ÉEÉBÉìÉhÉEçÏê¨é∏îs");
		return 0;
	}
	while (true)
	{
		if (!window.ProcessMessage()) {
			break;
		}
		bool result = window.ScreenFlip();
		if (!result) {
			assert(false && "îΩì]Ç…é∏îsÇµÇ‹ÇµÇΩ");
			break;
		}
	}
	return 1;
}

