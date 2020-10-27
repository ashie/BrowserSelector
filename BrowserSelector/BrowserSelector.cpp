#include "stdafx.h"
#include "BrowserSelector.h"
#include "BrowserSelectorCommon.h"

using namespace std;

static bool OpenByIE(BrowserSelector &app,  const wstring &url)
{
	bool succeeded = app.OpenByExistingIE(url);
	if (!succeeded) {
		HINSTANCE hInstance = ::ShellExecute(NULL, _T("open"), _T("iexplore.exe"), url.c_str(), NULL, SW_SHOW);
		succeeded = reinterpret_cast<int>(hInstance) > 32;
		if (!succeeded)
			DebugLog(L"Failed to launch IE: code=%d, url=%ls", hInstance, url.c_str());
	}
	return succeeded;
}

static int DumpConfig(const wstring &path)
{
	Config config;
	std::wstring buf;
	FILE *fp = NULL;

	config.LoadAll();
	config.dumpAsJson(buf);

	errno_t err = _wfopen_s(&fp, path.c_str(), L"w,ccs=UTF-8");
	if (err || !fp)
		return -1;

	fwrite(buf.c_str(), sizeof(wchar_t), buf.size(), fp);
	fclose(fp);
	return 0;
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
	wstring dumpPath;
	bool enableIEFallback = true;

	if (lpCmdLine && *lpCmdLine) {
		int nArgs = 0;
		LPWSTR *args = ::CommandLineToArgvW(lpCmdLine, &nArgs);
		// Pick up only last URL and last browser name.
		// Others are discarded.
		for (int i = 0; i < nArgs; i++) {
			wstring prefix(L"--browser=");
			wstring dumpPrefix(L"--dump-config=");
			wstring arg(args[i]);
			if (arg.find(prefix) == 0) {
				browserName = arg.substr(prefix.size());
				enableIEFallback = false;
			} else if (arg.find(dumpPrefix) == 0) {
				dumpPath = arg.substr(dumpPrefix.size());
			} else {
				url = arg;
			}
		}
		LocalFree(args);
	}

	if (!dumpPath.empty())
		return DumpConfig(dumpPath);

	Config config;
	config.LoadAll();
	BrowserSelector app(config);

	if (browserName.empty())
		browserName = app.GetBrowserNameToOpenURL(url);

	bool succeeded = false;
	if (browserName == L"ie") {
		succeeded = OpenByIE(app, url);
	} else {
		bool succeeded = app.OpenByModernBrowser(browserName, url);
		if (!succeeded && enableIEFallback)
			succeeded = OpenByIE(app, url);
	}

	return succeeded ? 0 : 1;
}
