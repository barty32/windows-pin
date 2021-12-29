
#include <stdio.h>
#include <Windows.h>
#include "PinDll.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
	int argc;
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	if(!argv || !argc || wcscmp(argv[0], L"--handle")){
		printf("This file is for Windows Pin internal usage only.");
		LocalFree(argv);
		return 2;
	}

	HANDLE hProcess = (HANDLE)wcstoll(argv[1], nullptr, 10);
	LocalFree(argv);
	if(!hProcess){
		return 3;
	}

	HMODULE hDll = LoadLibraryW(L"PinDll64");
	if(!hDll){
		return 4;
	}

	HOOKPROC proc = (HOOKPROC)GetProcAddress(hDll, "ExportHookProc");
	if(!proc){
		FreeLibrary(hDll);
		return 5;
	}

	// Inject 64-bit DLL to all processes
	HHOOK hhook = SetWindowsHookExW(WH_GETMESSAGE, proc, hDll, 0);
	if(!hhook){
		FreeLibrary(hDll);
		return 6;
	}

	// Wait until parent process terminates
	WaitForSingleObject(hProcess, INFINITE);

	UnhookWindowsHookEx(hhook);
	FreeLibrary(hDll);
	return 0;
}