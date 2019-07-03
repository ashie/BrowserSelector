// dllmain.cpp : DllMain の実装

#include "stdafx.h"
#include "resource.h"
#include "BrowserSelectorBHO_i.h"
#include "dllmain.h"

CBrowserSelectorBHOModule _AtlModule;

// DLL エントリ ポイント
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	if (dwReason == DLL_PROCESS_ATTACH) {
		TCHAR loader[MAX_PATH];
		GetModuleFileName(NULL, loader, MAX_PATH);
		if (_tcsicmp(loader, _T("explorer.exe"))) {
			return FALSE;
		}
	}
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
