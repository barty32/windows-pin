// Windows Pin.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Windows Pin.h"
#include "WindowsPinDll.h"

#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HHOOK hHook = NULL;
HHOOK hHook64 = NULL;
HHOOK hHookCbt = NULL;

BOOL bPump = TRUE;


// ------------------------------------------------------------------------------------------------
// for use with LoadLibrary and GetProcAddress
//
typedef void(_stdcall* UnsetMsgHookT)();
typedef int(_stdcall* SetMsgHookT)();

UnsetMsgHookT pfnUnsetMsgHook;
SetMsgHookT pfnSetMsgHook;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDC_WINDOWSPIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)){
		return FALSE;
	}

	HACCEL hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_WINDOWSPIN));

	MSG msg;

	// Main message loop:
	while (GetMessageW(&msg, NULL, 0, 0)){
		if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg)){
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance){
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_WINDOWSPIN));
	wcex.hCursor        = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPIN);
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm        = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_WINDOWSPIN));
	return RegisterClassExW(&wcex);
}




//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowExW(
		0,
		szWindowClass,
		L"Title",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if(!hWnd){
		return FALSE;
	}

	NOTIFYICONDATAW nidApp;
	nidApp.cbSize = sizeof(NOTIFYICONDATAW); // sizeof the struct in bytes 
	nidApp.hWnd = (HWND)hWnd;              //handle of the window which will process this app. messages 
	nidApp.uID = IDI_WINDOWSPIN;           //ID of the icon that will appear in the system tray 
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nidApp.hIcon = LoadIconW(hInstance, (LPCWSTR)MAKEINTRESOURCEW(IDI_WINDOWSPIN)); // handle of the Icon to be displayed, obtained from LoadIcon 
	nidApp.uCallbackMessage = WM_USER_SHELLICON;
	LoadStringW(hInstance, IDS_APPTOOLTIP, nidApp.szTip, MAX_LOADSTRING);
	Shell_NotifyIconW(NIM_ADD, &nidApp);


	// obtain msghook library functions

	HINSTANCE g_hInstLib = LoadLibraryW(L"WindowPinDll.dll");
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

	pfnSetMsgHook();

	//(SetMsgHookT)GetProcAddress(g_hInstLib, "SetMsgHook");


	//hHook = SetWindowsHookExW(WH_CALLWNDPROC, /*(HOOKPROC)*/HookProc/*GetProcAddress(g_hInstLib, "HookProc")*/, g_hInstLib/*GetModuleHandleW(L"WindowPinDll.dll")*/, 0);
	//if(hHook == NULL){
	//	int error = GetLastError();
	//}

	//hHook64 = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)HookProc, GetModuleHandleW(L"WindowPinDll64"), 0);
	//hHookCbt = SetWindowsHookExW(WH_CBT, (HOOKPROC)/*CBTProc*/GetProcAddress(g_hInstLib, "CBTProc"), g_hInstLib/*GetModuleHandleW(L"WindowPinDll.dll")*/, 0);

	bPump = TRUE;
	MSG msg;
	while(bPump){
		// Keep pumping...
		PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		Sleep(10);
	}

	//int error = GetLastError();
	//WinExec("notepad.exe", 1);

	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	return TRUE;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId){
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				bPump = FALSE;
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_USER_SHELLICON:
		// systray msg callback 
		switch(LOWORD(lParam))		{
			case WM_RBUTTONDOWN:
				POINT lpClickPoint;
				UINT uFlag = MF_BYPOSITION | MF_STRING;
				GetCursorPos(&lpClickPoint);
				HMENU hPopMenu = CreatePopupMenu();
				InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_ABOUT, L"About");
				//if(bDisable == TRUE)				{
				//	uFlag |= MF_GRAYED;
				//}
				//InsertMenu(hPopMenu, 0xFFFFFFFF, uFlag, IDM_TEST2, _T("Test 2")); // Test 2
				//InsertMenu(hPopMenu, 0xFFFFFFFF, uFlag, IDM_TEST1, _T("Test 1")); // Test 1				
				//InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, IDM_SEP, _T("SEP"));
				//if(bDisable == TRUE)				{
				//	InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_ENABLE, _T("Enable"));
				//}
				//else				{
				//	InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_DISABLE, _T("Disable"));
				//}
				InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, 0, NULL);
				InsertMenuW(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");

				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
				return TRUE;

		}
		break;
	case WM_DESTROY:
		UnhookWindowsHookEx(hHook);
		//UnhookWindowsHookEx(hHook64);
		UnhookWindowsHookEx(hHookCbt);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(lParam);
	switch (message){
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
