// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#define PINDLL
#include "PinDll.h"
#include <stdlib.h>


static HINSTANCE g_hDll = NULL;
static HHOOK hHook = NULL;
static HHOOK hCBTHook = NULL;

void UnsetHooks();
int  SetHooks();
void UpdateMenu(HMENU hMenu, HWND hWnd);
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
			g_hDll = hModule;
			SetHooks();
			break;
		case DLL_PROCESS_DETACH:
			UnsetHooks();
			break;
	}
	return TRUE;
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(nCode >= 0){
		LPCWPSTRUCT msg = (LPCWPSTRUCT)lParam;
		switch(msg->message){
			case WM_SYSCOMMAND:
			case WM_CONTEXTMENU:
			case WM_INITMENU:
			{
				//MENUITEMINFOW mItem;
				
				//MessageBoxW(((LPCWPSTRUCT)lParam)->hwnd, L"Context menu", L"Info", MB_ICONINFORMATION | MB_OK);
				HMENU sysmenu = GetSystemMenu(msg->hwnd, FALSE);
				UpdateMenu(sysmenu, msg->hwnd);

				//mItem.cbSize = sizeof(LPMENUITEMINFOW);
				//mItem.fMask = MIIM_STRING | MIIM_ID;
				//mItem.wID = 1183;
				//mItem.dwTypeData = (LPWSTR)L"Item";

				//AppendMenuW(sysmenu, MF_STRING, 1183, L"Menu Item");
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(nCode == HCBT_SYSCOMMAND){
		if(wParam == IDM_PINTOTOP){
			HWND hwndWindow = GetForegroundWindow();
			LONG dwExStyle = GetWindowLongPtrW(hwndWindow, GWL_EXSTYLE);
			HWND hInsertAfter = NULL;
			if(dwExStyle & WS_EX_TOPMOST){
				dwExStyle &= ~WS_EX_TOPMOST;
				hInsertAfter = HWND_NOTOPMOST;
				ModifyMenuW(GetSystemMenu(hwndWindow, FALSE), IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, L"Pin to top");
#ifdef _DEBUG
				MessageBoxW(GetForegroundWindow(), L"Window unpinned", L"Info", MB_ICONINFORMATION | MB_OK);
#endif
			}
			else{
				dwExStyle |= WS_EX_TOPMOST;
				hInsertAfter = HWND_TOPMOST;
				ModifyMenuW(GetSystemMenu(hwndWindow, FALSE), IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, L"Unpin from top");
#ifdef _DEBUG
				MessageBoxW(GetForegroundWindow(), L"Window pinned", L"Info", MB_ICONINFORMATION | MB_OK);
#endif
			}
			SetWindowLongPtrW(hwndWindow, GWL_EXSTYLE, dwExStyle);
			SetWindowPos(hwndWindow, hInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT PINDLL_API CALLBACK ExportHookProc(int nCode, WPARAM wParam, LPARAM lParam){
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


void UpdateMenu(HMENU hMenu, HWND hWnd){
	BOOL fInsertMenu = TRUE;
	LPCWSTR szMenuItemText;
	LONG exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
	if(exStyle & WS_EX_TOPMOST){
		szMenuItemText = L"Unpin from top";
	}
	else{
		szMenuItemText = L"Pin to top";
	}

	for(int i = 0; i < GetMenuItemCount(hMenu); i++){
		if(GetMenuItemID(hMenu, i) == IDM_PINTOTOP){
			//DeleteMenu(sysmenu, 20000, MF_BYCOMMAND);
			fInsertMenu = FALSE;
			break;
		}
	}
	if(fInsertMenu){
		//InsertMenuW(sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_STRING, IDM_PINTOTOP, szMenuItemText);
		//InsertMenuW(sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
		//InsertMenuW(sysmenu, IDM_PINTOTOP, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

		InsertMenuW(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_STRING, IDM_PINTOTOP, szMenuItemText);
		InsertMenuW(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, IDM_SEP, NULL);
	}
	else{
		ModifyMenuW(hMenu, IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, szMenuItemText);
	}
	//if(!InsertMenuItemW(sysmenu, /*SC_SIZE*/1, TRUE, &mItem)){
	//}
}



// ---------------------------------------------------------------------------
// Set the hook.
// If it is already set, unset it first.
//
BOOL SetHooks(){

	// verify the hook is not set
	UnsetHooks();
	DWORD dwThread = GetCurrentThreadId();

	// set the hook
	hHook = SetWindowsHookExW(WH_CALLWNDPROC, HookProc, NULL, dwThread);
	hCBTHook = SetWindowsHookExW(WH_CBT, CBTProc, NULL, dwThread);
	//int error = GetLastError();
	//MessageBoxW(NULL, L"Set Hook", L"", MB_ICONEXCLAMATION);

	return 1;
}


// ---------------------------------------------------------------------------
// Unset the hook
//
void UnsetHooks(){
	UnhookWindowsHookEx(hHook);
	UnhookWindowsHookEx(hCBTHook);
}

