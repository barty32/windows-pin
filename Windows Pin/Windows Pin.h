#pragma once

#include "resource.h"
#include "framework.h"

#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1

#ifdef _DEBUG
#define TRACE(_msg) OutputDebugStringW(_msg)
#else
#define TRACE(_msg)
#endif

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

NOTIFYICONDATAW* CreateTrayIcon(HWND hWnd);
bool Is64BitWindows();
void ErrorHandler(LPCWSTR errMsg, DWORD errCode = 0, DWORD dwType = MB_ICONERROR);


bool StartPin(HWND hWnd);
bool EndPin();
bool MovePin();

bool PinWindow(HWND hWnd);
bool UnpinWindow(HWND hWnd);

bool HighlightWindow(HWND hWnd);
HWND FindParent(HWND hWnd);
bool CheckWindowValidity(HWND hWnd);