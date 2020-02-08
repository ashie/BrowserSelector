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

	wstring url;
	wstring browserName;

	if (lpCmdLine && *lpCmdLine) {
		int nArgs = 0;
		LPWSTR *args = ::CommandLineToArgvW(lpCmdLine, &nArgs);
		// Pick up only last URL and last browser name.
		// Others are discarded.
		for (int i = 0; i < nArgs; i++) {
			wstring prefix(L"--browser=");
			wstring arg(args[i]);
			if (arg.find(prefix) == 0) {
				browserName = arg.substr(prefix.size());
			} else {
				url = arg;
			}
		}
		LocalFree(args);
	}

	if (browserName.empty()) {
		Config config;
		config.LoadAll();
		browserName = ::GetBrowserNameToOpenURL(url, config);
	}

	bool openByIE = true;
	if (browserName != L"ie")
		openByIE = !OpenByModernBrowser(browserName, url);
	if (openByIE)
		OpenByIE(url);

	return 0;
}
