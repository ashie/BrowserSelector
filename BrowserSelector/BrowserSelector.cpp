#include "stdafx.h"
#include "BrowserSelector.h"
#include "BrowserSelectorCommon.h"

using namespace std;

static void OpenByIE(const wstring &url)
{
	bool succeeded = OpenByExistingIE(url);
	if (!succeeded)
		::ShellExecute(NULL, _T("open"), _T("iexplore.exe"), url.c_str(), NULL, SW_SHOW);
}

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	Config config;
	config.LoadAll();

	wstring url;
	if (lpCmdLine && *lpCmdLine) {
		int nArgs = 0;
		LPWSTR *args = ::CommandLineToArgvW(lpCmdLine, &nArgs);
		if (nArgs >= 1)
			url = args[0];
		LocalFree(args);
	}

	wstring browserName = ::GetBrowserNameToOpenURL(url, config);

	bool openByIE = true;
	if (browserName != L"ie")
		openByIE = !OpenByModernBrowser(browserName, url);
	if (openByIE)
		OpenByIE(url);

	return 0;
}
