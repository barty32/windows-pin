#pragma once
#include <windows.h>

#ifdef PINDLL
#define PINDLL_API __declspec(dllexport)
#else
#define PINDLL_API __declspec(dllimport)
#endif

#define IDM_PINTOTOP 20000
#define IDM_SEP IDM_PINTOTOP+1

#ifdef __cplusplus
extern "C" {
#endif


//LRESULT WINDOWSPINDLL_API HookProc(int nCode, WPARAM wParam, LPARAM lParam);
//LRESULT WINDOWSPINDLL_API CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT PINDLL_API CALLBACK ExportHookProc(int nCode, WPARAM wParam, LPARAM lParam);




#ifdef __cplusplus
}
#endif