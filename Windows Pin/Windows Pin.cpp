// Windows Pin.cpp : Defines the entry point for the application.
//

#include "Windows Pin.h"
#include "PinDll.h"

bool    g_bPinning = false;
HCURSOR g_hPinCursor = NULL;
std::list<HWND> g_pinnedWnds;

// Global Variables:
HINSTANCE  hInst;
HWND       hWndMain;
const UINT WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated");
HHOOK      hHook = NULL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;
	g_hPinCursor = LoadCursorW(hInst, MAKEINTRESOURCEW(IDI_PIN_CURSOR));

	// Register main window class (hidden)
	WNDCLASSEXW wcex = {0};
	wcex.cbSize        = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc   = WndProc;
	wcex.hInstance     = hInstance;
	wcex.lpszClassName = L"WindowsPinWndClass";
	if(!RegisterClassExW(&wcex)){
		ErrorHandler(L"Class registration failed", GetLastError());
		return false;
	}

	hWndMain = CreateWindowExW(WS_EX_LAYERED, L"WindowsPinWndClass", L"Windows Pin", WS_POPUP, 0, 0, 0, 0, 0, 0, hInstance, 0);
	if(!hWndMain){
		ErrorHandler(L"Window creation failed", GetLastError());
		return false;
	}

	NOTIFYICONDATAW* nid = CreateTrayIcon(hWndMain);

	if(!Shell_NotifyIconW(NIM_ADD, (PNOTIFYICONDATAW)GetPropW(hWndMain, L"trayIcon"))){
		ErrorHandler(L"Tray icon creation failed", GetLastError());
		return false;
	}

	// Prepare tray popup menu
	HMENU hPopMenu = CreatePopupMenu();
	InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_ABOUT, L"About");
	InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_PIN, L"Pin a window");
	InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_UNPIN, L"Unpin all pinned windows");
	InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, 0, NULL);
	InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");
	SetPropW(hWndMain, L"hPopMenu", hPopMenu);

	// Inject DLL to all 32-bit processes
	hHook = SetWindowsHookExW(WH_GETMESSAGE, ExportHookProc, GetModuleHandleW(L"PinDll32"), 0);
	if(!hHook){
		ErrorHandler(L"32-bit hooking failed", GetLastError());
		return false;
	}
	
	HANDLE currentProcess = nullptr;
	if(Is64BitWindows()){
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		currentProcess = OpenProcess(SYNCHRONIZE, TRUE, GetCurrentProcessId());

		std::wstring cmd = L"Inject64.exe --handle " + std::to_wstring((int)currentProcess);

		// Start the child process. 
		if(!CreateProcessW(nullptr,
			(LPWSTR)cmd.data(),
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			TRUE,           // Handle inheritance
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)            // Pointer to PROCESS_INFORMATION structure
			){
			ErrorHandler(L"64-bit hook execution failed", GetLastError());
			return false;
		}
		WaitForSingleObject(pi.hProcess, 500);
		DWORD dwExitCode;
		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		if(dwExitCode != STILL_ACTIVE && dwExitCode > 0){
			ErrorHandler(L"64-bit hook error", dwExitCode);
			return false;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	MSG msg;
	while(GetMessageW(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	delete nid;
	DestroyMenu(hPopMenu);
	if(currentProcess){ 
		CloseHandle(currentProcess); 
	}
	//UnhookWindowsHookEx(hook);

	return (int)msg.wParam;
}

//LRESULT CALLBACK LowLevelMouseProc(
//	_In_ int    nCode,
//	_In_ WPARAM wParam,
//	_In_ LPARAM lParam
//){
//	static bool bCaptured = false;
//	if(nCode >= HC_ACTION){
//		LPMSLLHOOKSTRUCT mss = (LPMSLLHOOKSTRUCT)lParam;
//		if(g_bPinning){
//			switch(wParam){
//				case WM_LBUTTONDOWN:
//				if(!bCaptured){
//					//CallNextHookEx(NULL, nCode, WM_LBUTTONDOWN, lParam);
//					//DefWindowProcW(hWndMain, WM_LBUTTONDOWN, 0, MAKELPARAM(mss->pt.x, mss->pt.y));
//					//SetCursorPos(mss->pt.x, mss->pt.y);
//					SetCursor(g_hPinCursor);
//					SetCapture(hWndMain);
//					bCaptured = true;
//					return false;
//				}
//				//case WM_LBUTTONDOWN:
//				//case WM_MOUSEMOVE:
//					//SetCursorPos(100, 100);
//
//					//SetCapture(hWndMain);
//					//return true;
//
//			}
//		}
//	}
//	return CallNextHookEx(NULL, nCode, wParam, lParam);
//}


bool StartPin(HWND hWnd){
	g_bPinning = true;
	POINT pos;
	GetCursorPos(&pos);

	// Hacky solution to set mouse capture without clicking
	SetWindowPos(hWnd, HWND_TOPMOST, pos.x - 10, pos.y - 10, 20, 20, SWP_SHOWWINDOW);
	SetLayeredWindowAttributes(hWnd, RGB(255, 0, 0), 200, LWA_COLORKEY | LWA_ALPHA);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	SetCursor(LoadCursorW(hInst, MAKEINTRESOURCEW(IDI_PIN_CURSOR)));
	SetCapture(hWnd);
	SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
	return true;
}


bool EndPin(){
	g_bPinning = false;

	// Determine the window that lies underneath the mouse cursor.
	POINT pt;
	GetCursorPos(&pt);
	HWND hWnd = FindParent(WindowFromPoint(pt));

	ReleaseCapture();
	InvalidateRect(NULL, NULL, FALSE);

	if(CheckWindowValidity(hWnd)){
		//WCHAR title[120];
		//GetWindowTextW(hWnd, title, 120);
		//WCHAR result[200];
		//_snwprintf_s(result, 200, L"Handle: 0x%08X\nTitle: %s", (int)hWnd, title);
		//MessageBoxW(NULL, result, L"Info", MB_ICONINFORMATION);
		PinWindow(hWnd);
		return true;
	}
	return false;
}

bool MovePin(){
	static HWND lastWnd = NULL;
	POINT pt;
	GetCursorPos(&pt);

	// Determine the window that lies underneath the mouse cursor.
	HWND hWnd = FindParent(WindowFromPoint(pt));
	if(lastWnd == hWnd){
		return false;
	}

	// If there was a previously found window, we must instruct it to refresh itself. 
	if(lastWnd){
		InvalidateRect(NULL, NULL, TRUE);
		UpdateWindow(lastWnd);
		RedrawWindow(lastWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	// Indicate that this found window is now the current found window.
	lastWnd = hWnd;

	// Check first for validity.
	if(CheckWindowValidity(hWnd)){
		return HighlightWindow(hWnd);
	}
	return false;
}

bool HighlightWindow(HWND hWnd){

	HDC hdcScreen = GetWindowDC(NULL);
	if(!hdcScreen){
		TRACE(L"Highligt window failed - HDC is null");
		return false;
	}

	RECT rcWnd;
	if(FAILED(DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rcWnd, sizeof(rcWnd)))){
		//TRACE(L"Highligt window failed - GetWindowAttr failed");
		GetWindowRect(hWnd, &rcWnd);
	}

	HPEN hPen = CreatePen(PS_SOLID, 10, RGB(255, 0, 0));
	HGDIOBJ oldBrush = SelectObject(hdcScreen, GetStockObject(HOLLOW_BRUSH));
	HGDIOBJ oldPen = SelectObject(hdcScreen, hPen);

	Rectangle(hdcScreen, rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

	// Cleanup
	SelectObject(hdcScreen, oldBrush);
	SelectObject(hdcScreen, oldPen);
	DeleteObject(hPen);
	ReleaseDC(NULL, hdcScreen);
	return true;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	
	if(message == WM_TASKBARCREATED){
		Shell_NotifyIconW(NIM_ADD, (PNOTIFYICONDATAW)GetPropW(hWnd, L"trayIcon"));
	}
	switch(message){
		case WM_COMMAND:
			// Parse the menu selections:
			switch(LOWORD(wParam)){
				case IDM_PIN:
					//PinActiveWindow(hWnd);
					StartPin(hWnd);
					break;
				case IDM_UNPIN:
					for(auto wnd : g_pinnedWnds){
						UnpinWindow(wnd);
					}
					g_pinnedWnds.clear();
					break;
				case IDM_ABOUT:
					return DialogBoxParamW(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, About, 0);
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		break;
		case WM_LBUTTONUP:
			if(g_bPinning){
				EndPin();
				return false;
			}
			break;
		case WM_MOUSEMOVE:
			if(g_bPinning){
				MovePin();
				return false;
			}
			break;
		case WM_USER_SHELLICON:
			switch(LOWORD(lParam)){
				case WM_RBUTTONUP:
				{
					POINT lpClickPoint;
					UINT uFlag = MF_BYPOSITION | MF_STRING;
					GetCursorPos(&lpClickPoint);
					TrackPopupMenu((HMENU)GetPropW(hWnd, L"hPopMenu"), TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
					return true;
				}
				case WM_LBUTTONUP:
					StartPin(hWnd);
					return false;
			}
			break;
		case WM_DESTROY:
			UnhookWindowsHookEx(hHook);
			Shell_NotifyIconW(NIM_DELETE, (PNOTIFYICONDATAW)GetPropW(hWnd, L"trayIcon"));
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}


bool PinWindow(HWND hWnd){
	LONG dwExStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
	dwExStyle |= WS_EX_TOPMOST;
	SetWindowLongW(hWnd, GWL_EXSTYLE, dwExStyle);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_SHOWWINDOW);
	g_pinnedWnds.push_back(hWnd);
	return true;
}

bool UnpinWindow(HWND hWnd){
	LONG dwExStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
	dwExStyle &= ~WS_EX_TOPMOST;
	SetWindowLongW(hWnd, GWL_EXSTYLE, dwExStyle);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_SHOWWINDOW);
	return true;
}

NOTIFYICONDATAW* CreateTrayIcon(HWND hWnd){
	// Create tray icon
	NOTIFYICONDATAW* nidApp = new NOTIFYICONDATAW;
	if(!nidApp) return nullptr;
	nidApp->cbSize = sizeof(NOTIFYICONDATAW);
	nidApp->hWnd = hWnd;                      //handle of the window which will process this app. messages 
	nidApp->uID = IDI_WINDOWSPIN;             //ID of the icon that will appear in the system tray 
	nidApp->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nidApp->hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_WINDOWSPIN));
	nidApp->uCallbackMessage = WM_USER_SHELLICON;
	LoadStringW(hInst, IDS_APPTOOLTIP, nidApp->szTip, _countof(nidApp->szTip));
	SetPropW(hWnd, L"trayIcon", nidApp);
	return nidApp;
}

bool CheckWindowValidity(HWND hWnd){
	if(!hWnd || !IsWindow(hWnd) || hWnd == GetShellWindow() || hWnd == FindWindowW(L"Shell_TrayWnd", NULL)){
		return false;
	}
	return true;
}

HWND FindParent(HWND hWnd){
	DWORD dwStyle = GetWindowLongW(hWnd, GWL_STYLE);
	if(dwStyle & WS_CHILD){
		return FindParent(GetParent(hWnd));
	}
	return hWnd;
}


bool Is64BitWindows(){
#if defined(_WIN64)
	return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows, so must sniff
	BOOL f64 = FALSE;
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
	return false; // Win64 does not support Win16
#endif
}


void ErrorHandler(LPCWSTR errMsg, DWORD errCode, DWORD dwType){
	std::wstring msg(errMsg);
	if(errCode){
		msg.append(L"\nError code: " + std::to_wstring(errCode));
	}
	MessageBoxW(NULL, msg.c_str(), L"Windows Pin - Error", dwType | MB_OK);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(lParam);
	switch (message){
	case WM_INITDIALOG:
		return true;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, LOWORD(wParam));
			return true;
		}
		break;
	}
	return false;
}



//void PinActiveWindow(HWND hWndApp){
//	HWND hwndWindow = GetWindow(GetDesktopWindow(), GW_HWNDFIRST);// GetForegroundWindow();
//	LONG dwExStyle = GetWindowLongPtrW(hwndWindow, GWL_EXSTYLE);
//	HWND hInsertAfter = NULL;
//	if(dwExStyle & WS_EX_TOPMOST){
//		dwExStyle &= ~WS_EX_TOPMOST;
//		hInsertAfter = HWND_NOTOPMOST;
//		ModifyMenuW((HMENU)GetPropW(hWndApp, L"hPopMenu"), IDM_PIN, MF_BYCOMMAND, IDM_PIN, L"Pin current window");
//	}
//	else{
//		dwExStyle |= WS_EX_TOPMOST;
//		hInsertAfter = HWND_TOPMOST;
//		ModifyMenuW((HMENU)GetPropW(hWndApp, L"hPopMenu"), IDM_PIN, MF_BYCOMMAND, IDM_PIN, L"Unpin current window");
//	}
//	SetWindowLongPtrW(hwndWindow, GWL_EXSTYLE, dwExStyle);
//	SetWindowPos(hwndWindow, hInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_SHOWWINDOW);
//}

//---------------------------------------------------------------------------------------------InitInstance
//BOOL InitInstance(HINSTANCE hInstance){
	// obtain msghook library functions

	/*HINSTANCE g_hInstLib = LoadLibraryW(L"WindowPinDll.dll");
	if(g_hInstLib == NULL){
		return FALSE;
	}

	pfnSetMsgHook = (SetMsgHookT)GetProcAddress(g_hInstLib, "SetMsgHook");
	if(NULL == pfnSetMsgHook){
		FreeLibrary(g_hInstLib);
		return FALSE;
	}

	pfnUnsetMsgHook = (UnsetMsgHookT)GetProcAddress(g_hInstLib, "UnsetMsgHook");
	if(NULL == pfnUnsetMsgHook){
		FreeLibrary(g_hInstLib);
		return FALSE;
	}

	pfnSetMsgHook();*/
	//(SetMsgHookT)GetProcAddress(g_hInstLib, "SetMsgHook");
	//hHook = SetWindowsHookExW(WH_CALLWNDPROC, /*(HOOKPROC)*/HookProc/*GetProcAddress(g_hInstLib, "HookProc")*/, g_hInstLib/*GetModuleHandleW(L"WindowPinDll.dll")*/, 0);
	//if(hHook == NULL){
	//	int error = GetLastError();
	//}
	//hHook64 = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)HookProc, GetModuleHandleW(L"WindowPinDll64"), 0);
	//hHookCbt = SetWindowsHookExW(WH_CBT, (HOOKPROC)/*CBTProc*/GetProcAddress(g_hInstLib, "CBTProc"), g_hInstLib/*GetModuleHandleW(L"WindowPinDll.dll")*/, 0);
	//bPump = TRUE;
	//MSG msg;
	//while(bPump){
	//	// Keep pumping...
	//	PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
	//	TranslateMessage(&msg);
	//	DispatchMessageW(&msg);
	//	Sleep(10);
	//}
	//int error = GetLastError();
	//WinExec("notepad.exe", 1);
//}


/*
bool HighlightWindow(HWND hWndIn){
	HWND hWnd = hWndIn;//FindParent(hWndIn);

	//OutputDebugStringW(std::format(L"Highlighting current window {}\n", (int)hWndIn).c_str());


	HDC hdcScreen = GetWindowDC(NULL);
	if(!hdcScreen){
		ErrorHandler(L"DC is null", GetLastError());
		return false;
	}

	RECT rcWnd;
	//GetWindowRect(hWnd, &rcWnd);
	DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rcWnd, sizeof(rcWnd));

	//RECT rcScreen = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	//int scrW = GetSystemMetrics(SM_CXSCREEN);
	//int scrH = GetSystemMetrics(SM_CYSCREEN);



	//RedrawWindow(GetDesktopWindow(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	//if(bRefresh){
	//	InvalidateRect(NULL, NULL, false);
	//}

	//WCHAR ss[100]; 
	//GetWindowTextW(hWnd, ss, 100);

	//HDC tempDC = CreateCompatibleDC(NULL);
	//BitBlt(tempDC, 0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, hdc, )
	//ExcludeClipRect(hdc, rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

	//HDC tmpDC = CreateCompatibleDC(g_hdcScreen);
	//HBITMAP hBmp = CreateCompatibleBitmap(tmpDC, scrW, scrH);
	//SelectObject(tmpDC, hBmp);
	//BitBlt(tmpDC, 0, 0, scrW, scrH, g_hdcScreen, 0, 0, SRCCOPY);

	//FillRect(tmpDC, &rcWnd, (HBRUSH)CreateSolidBrush(RGB(255, 255, 255)));
	//FillRect(tmpDC, &rcWnd, (HBRUSH)GetStockObject(HOLLOW_BRUSH));



	HGDIOBJ oldBrush = SelectObject(hdcScreen, GetStockObject(HOLLOW_BRUSH));
	HGDIOBJ oldPen = SelectObject(hdcScreen, CreatePen(PS_SOLID, 10, RGB(255, 0, 0)));
	////Rectangle(hdc, 0, 0, scrW, scrH);
	Rectangle(hdcScreen, rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);


	//InvalidateRect(hWnd, NULL, FALSE);

	//FillRect(hdc, &rcWnd, (HBRUSH)GetStockObject(HOLLOW_BRUSH));

	//SelectObject(hdc, )
	//FillRect(hdc, &rcScreen, );

	//BLENDFUNCTION bf = {0};
	//bf.SourceConstantAlpha = 200;
	//bf.AlphaFormat = AC_SRC_ALPHA;
	//if(!AlphaBlend(hdc, 0, 0, scrW, scrH, tmpDC, 0, 0, scrW, scrH, bf)){
	//	OutputDebugStringW(L"AplhaBlend failed");
	//}


	//TransparentBlt(hdcScreen, 0, 0, scrW, scrH, tmpDC, 0, 0, scrW, scrH, RGB(255, 255, 255));

	SelectObject(hdcScreen, oldBrush);
	SelectObject(hdcScreen, oldPen);

	ReleaseDC(NULL, hdcScreen);
	return true;
}
*/



// Create snapshot of current screen
	//int sw = GetSystemMetrics(SM_CXSCREEN);
	//int sy = GetSystemMetrics(SM_CYSCREEN);
	//HDC hdc = GetWindowDC(NULL);
	//g_hdcScreen = CreateCompatibleDC(hdc);
	//HBITMAP hBmp = CreateCompatibleBitmap(hdc, sw, sy);
	//SelectObject(g_hdcScreen, hBmp);
	//BitBlt(g_hdcScreen, 0, 0, sw, sy, hdc, 0, 0, SRCCOPY);
	//ReleaseDC(NULL, hdc);

//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hdc = BeginPaint(hWnd, &ps);
		//	RECT rc;
		//	GetWindowRect(hWnd, &rc);
		//	FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		//	EndPaint(hWnd, &ps);
		//	return false;
		//}