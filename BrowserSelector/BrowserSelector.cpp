#include "stdafx.h"
#include "BrowserSelector.h"
#include <string>
#include <vector>
#include <ShellAPI.h>
#include <DbgHelp.h>

using namespace std;

static void LoadURLPatterns(vector<wstring> &urlPatterns, bool systemWide = false)
{
	CRegKey reg;

	LONG result = reg.Open(
		systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
		_T("SOFTWARE\\ClearCode\\BrowserSelector\\IntranetURLPatterns"),
		KEY_READ);

	for (DWORD idx = 0; result == ERROR_SUCCESS; idx++) {
		TCHAR value[256];
		DWORD valueLen = 256;
		result = ::RegEnumValue(reg.m_hKey, idx,value, &valueLen, NULL, NULL, NULL, NULL);
		if (result != ERROR_SUCCESS)
			continue;
		urlPatterns.push_back(value);
	}

	reg.Close();
}

static bool IsIntranetURL(const wstring &url, const vector<wstring> &urlPatterns)
{
	if (url.empty())
		return false;

	static CComAutoCriticalSection symMatchSection;
	vector<wstring>::const_iterator it = urlPatterns.begin();

	for (; it != urlPatterns.end(); it++) {
		symMatchSection.Lock();
		BOOL matched = SymMatchStringW(url.c_str(), it->c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return true;
	}

	return false;
}

void OpenByIE(const wstring &url)
{
	::ShellExecute(NULL, _T("open"), _T("iexplore.exe"), url.c_str(), NULL, SW_SHOW);
}

void OpenByFirefox(const wstring &url)
{
	::ShellExecute(NULL, _T("open"), _T("firefox.exe"), url.c_str(), NULL, SW_SHOW);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	vector<wstring> urlPatterns;
	bool systemWide = true;
	LoadURLPatterns(urlPatterns, systemWide);
	LoadURLPatterns(urlPatterns);

	wstring url = lpCmdLine;

	if (IsIntranetURL(url, urlPatterns))
		OpenByIE(url);
	else
		OpenByFirefox(url);
	return 0;
}
