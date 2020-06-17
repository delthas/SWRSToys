#include <windows.h>
#include "APIHook.hpp"
#include <shlwapi.h>

/*
This is my first dive into WinAPI so a lot of this code is probably unnecessary.
*/

static WNDPROC s_origWndProc;

LRESULT CALLBACK OnWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	

	return s_origWndProc(hWnd, uMsg, wParam, lParam);
}

HWND(__stdcall *s_CreateWindowExA)
(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
	HINSTANCE hInstance, LPVOID lpParam);

HWND __stdcall OnCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {

	//Use WS_POPUP instead of WS_OVERLAPPED for borderless and hiding taskbar
	dwStyle = WS_POPUP;
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);

	//Get 4:3 width, otherwise the game would be stretched (Maybe there is some way to draw 16:9 with borders?)
	nWidth = ::MulDiv(w, 3, 4);
	//Center the game window, so center of screen - size of window /2 to get the starting x position of the window (because it draws towards the right)
	x = w / 2 - nWidth / 2;
	y = 0;
	RECT rct = { x, y, w, h };
	::AdjustWindowRectEx(&rct, dwStyle, FALSE, dwExStyle);
	nHeight = rct.bottom - rct.top;

	return ::CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

ATOM __stdcall OnRegisterClassExA(WNDCLASSEXA *pWcex) {
	s_origWndProc = pWcex->lpfnWndProc;
	pWcex->lpfnWndProc = OnWindowProc;
	return ::RegisterClassExA(pWcex);
}

//Set size of game rendering inside the window
//Anything higher than 640x480 will give you black boxes and you'd be able to see things popping in and out. So don't change it.
BOOL __stdcall OnGetWindowInfo(HWND hwnd, PWINDOWINFO pwi) {
	pwi->rcClient.left = pwi->rcClient.top = 0;
	pwi->rcClient.right = 640;
	pwi->rcClient.bottom = 480;
	return TRUE;
}


extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {

	HMODULE swrsModule = GetModuleHandle(NULL);
	//::HookAPICall(swrsModule, "KERNEL32.DLL", "ExitProcess", (FARPROC)OnExitProcess);
	::HookAPICall(swrsModule, "USER32.DLL", "RegisterClassExA", (FARPROC)OnRegisterClassExA);
	::HookAPICall(swrsModule, "USER32.DLL", "CreateWindowExA", (FARPROC)OnCreateWindowExA);
	//::HookAPICall(swrsModule, "USER32.DLL", "SetWindowPos", (FARPROC)OnSetWindowPos);
	::HookAPICall(swrsModule, "USER32.DLL", "GetWindowInfo", (FARPROC)OnGetWindowInfo);

	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}


/*
Written by Rhythm Lunatic (By copying code from WindowResizer module and hacking it until it worked)
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/