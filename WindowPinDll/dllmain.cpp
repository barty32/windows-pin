// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "WindowsPinDll.h"
#include <stdlib.h>


static HINSTANCE g_hDll = NULL; // needed to set the hook
HHOOK hHook = NULL;



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
			g_hDll = hModule;

			SetMsgHook();

			// set up the shared data in the memory mapped file

			// detemine if file mapping object already exists
			//g_hMemoryMappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,g_pszMemoryMappedFileName);

			// file mapping object exists so map a view of it
			//if(NULL != g_hMemoryMappedFile){
			//	g_pHookData = (HOOKDATA*)MapViewOfFile(
			//		g_hMemoryMappedFile,
			//		FILE_MAP_ALL_ACCESS, 0, 0,
			//		sizeof(*g_pHookData)
			//	);
			//	if(NULL == g_pHookData){
			//		// a major error occured so close up and get out
			//		CloseHandle(g_hMemoryMappedFile);
			//		g_hMemoryMappedFile = NULL;
			//		return FALSE;
			//	}
			//}
			//else{
			//	// file mapping object doesn't exist so create it
			//	SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, FALSE};
			//	g_hMemoryMappedFile = CreateFileMapping(
			//		INVALID_HANDLE_VALUE,
			//		&sa,
			//		PAGE_READWRITE,
			//		0,
			//		sizeof(*g_pHookData),
			//		g_pszMemoryMappedFileName
			//	);
			//
			//	if(NULL != g_hMemoryMappedFile){
			//
			//		g_pHookData = (HOOKDATA*)MapViewOfFile(
			//			g_hMemoryMappedFile,
			//			FILE_MAP_ALL_ACCESS, 0, 0,
			//			sizeof(*g_pHookData)
			//		);
			//
			//		if(NULL == g_pHookData){
			//			// a major error occured so close up and get out
			//			CloseHandle(g_hMemoryMappedFile);
			//			g_hMemoryMappedFile = NULL;
			//			return FALSE;
			//		}
			//
			//		// zero the newly create file mapping 
			//		ZeroMemory(g_pHookData, sizeof(*g_pHookData));
			//	}
			//}
			break;
		case DLL_PROCESS_DETACH:

			// unmap the memory mapped file and
			//if(NULL != g_pHookData){
			//	UnmapViewOfFile(g_pHookData);
			//	g_pHookData = NULL;
			//}
			//// close the file mapping handle
			//if(NULL != g_hMemoryMappedFile){
			//	CloseHandle(g_hMemoryMappedFile);
			//	g_hMemoryMappedFile = NULL;
			//}
			break;
	}
	return TRUE;
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(nCode >= 0){
		switch(((LPCWPSTRUCT)lParam)->message){
			case WM_SYSCOMMAND:
			case WM_CONTEXTMENU:
			case WM_INITMENU:
			{
				//MENUITEMINFOW mItem;
				BOOL fInsertMenu = TRUE;
				WCHAR szMenuItemText[15];
				//MessageBoxW(((LPCWPSTRUCT)lParam)->hwnd, L"Context menu", L"Info", MB_ICONINFORMATION | MB_OK);
				HMENU sysmenu = GetSystemMenu(((LPCWPSTRUCT)lParam)->hwnd, FALSE);

				//mItem.cbSize = sizeof(LPMENUITEMINFOW);
				//mItem.fMask = MIIM_STRING | MIIM_ID;
				//mItem.wID = 1183;
				//mItem.dwTypeData = (LPWSTR)L"Item";

				//AppendMenuW(sysmenu, MF_STRING, 1183, L"Menu Item");


				LONG exStyle = GetWindowLongPtrW(((LPCWPSTRUCT)lParam)->hwnd, GWL_EXSTYLE);
				if(exStyle & WS_EX_TOPMOST){
					wcscpy_s(szMenuItemText, _countof(szMenuItemText), L"Unpin from top");
				}
				else{
					wcscpy_s(szMenuItemText, _countof(szMenuItemText), L"Pin to top");
				}
				

				for(int i = 0; i < GetMenuItemCount(sysmenu); i++){
					if(GetMenuItemID(sysmenu, i) == IDM_PINTOTOP){
						//DeleteMenu(sysmenu, 20000, MF_BYCOMMAND);
						fInsertMenu = FALSE;
					}
				}
				if(fInsertMenu){
					//InsertMenuW(sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_STRING, IDM_PINTOTOP, szMenuItemText);
					//InsertMenuW(sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
					//InsertMenuW(sysmenu, IDM_PINTOTOP, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

					InsertMenuW(sysmenu, SC_CLOSE, MF_BYCOMMAND | MF_STRING, IDM_PINTOTOP, szMenuItemText);
					InsertMenuW(sysmenu, SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, IDM_SEP, NULL);
				}
				else{
					ModifyMenuW(sysmenu, IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, szMenuItemText);
				}
				//if(!InsertMenuItemW(sysmenu, /*SC_SIZE*/1, TRUE, &mItem)){
				//}
			}
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(nCode == HCBT_SYSCOMMAND){
		if(wParam == IDM_PINTOTOP){
			//MessageBoxW(GetForegroundWindow(), L"New item clicked", L"Info", MB_ICONINFORMATION | MB_OK);
			HWND hwndWindow = GetForegroundWindow();
			LONG dwExStyle = GetWindowLongPtrW(hwndWindow, GWL_EXSTYLE);
			HWND hInsertAfter = NULL;
			if(dwExStyle & WS_EX_TOPMOST){
				dwExStyle &= ~WS_EX_TOPMOST;
				hInsertAfter = HWND_NOTOPMOST;
				ModifyMenuW(GetSystemMenu(hwndWindow, FALSE), IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, L"Pin to top");
			}
			else{
				dwExStyle |= WS_EX_TOPMOST;
				hInsertAfter = HWND_TOPMOST;
				ModifyMenuW(GetSystemMenu(hwndWindow, FALSE), IDM_PINTOTOP, MF_BYCOMMAND, IDM_PINTOTOP, L"Unpin from top");
			}
			SetWindowLongPtrW(hwndWindow, GWL_EXSTYLE, dwExStyle);
			SetWindowPos(hwndWindow, hInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}



// ---------------------------------------------------------------------------
// Set the hook.
// If it is already set, unset it first.
//
BOOL __declspec(dllexport) WINAPI SetMsgHook(){

	// verify the hook is not set
	//UnsetMsgHook();

	// set the hook
	hHook = SetWindowsHookExW(WH_GETMESSAGE, HookProc, g_hDll, 0);
	int error = GetLastError();

	return 1;
}


// ---------------------------------------------------------------------------
// Unset the hook
//
void __declspec(dllexport) WINAPI UnsetMsgHook(){
	UnhookWindowsHookEx(hHook);
}

