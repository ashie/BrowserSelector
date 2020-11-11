#include "stdafx.h"
#include "resource.h"
#include "BrowserSelectorBHO_i.h"
#include "dllmain.h"
#include <string>

CBrowserSelectorBHOModule _AtlModule;

static std::wstring GetModuleFolderName(HINSTANCE hInstance)
{
	WCHAR buf[MAX_PATH];
	DWORD nWritten = ::GetModuleFileNameW(hInstance, buf, MAX_PATH);
	if (!nWritten)
		return L"";

	BOOL succeeded = ::PathRemoveFileSpecW(buf);
	if (!succeeded)
		return L"";

	::PathAddBackslashW(buf);

	return buf;
}

HRESULT CBrowserSelectorBHOModule::AddCommonRGSReplacements(IRegistrarBase *pRegistrar)
{
	std::wstring folderPath = ::GetModuleFolderName(ATL::_AtlBaseModule.GetModuleInstance());
	pRegistrar->AddReplacement(L"MODULE_FOLDER", folderPath.c_str());
	return __super::AddCommonRGSReplacements(pRegistrar);
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hInstance);
	if (dwReason == DLL_PROCESS_ATTACH) {
		TCHAR loader[MAX_PATH];
		GetModuleFileName(NULL, loader, MAX_PATH);
		if (!_tcsicmp(loader, _T("explorer.exe"))) {
			return FALSE;
		}
	}
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
