#pragma once
#include <windows.h>

#ifdef WINDOWSPINDLL
#define WINDOWSPINDLL_API __declspec(dllexport)
#else
#define WINDOWSPINDLL_API __declspec(dllimport)
#endif

#define IDM_PINTOTOP 20000
#define IDM_SEP IDM_PINTOTOP+1

#ifdef __cplusplus
extern "C" {
#endif


//LRESULT WINDOWSPINDLL_API HookProc(int nCode, WPARAM wParam, LPARAM lParam);
//LRESULT WINDOWSPINDLL_API CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
void		__declspec(dllexport)	WINAPI UnsetMsgHook();
int			__declspec(dllexport)	WINAPI SetMsgHook();
LRESULT CALLBACK	HookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	CBTProc(int nCode, WPARAM wParam, LPARAM lParam);


#ifdef __cplusplus
}
#endif