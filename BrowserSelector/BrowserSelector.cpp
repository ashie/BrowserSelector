#include "stdafx.h"
#include "BrowserSelector.h"
#include "BrowserSelectorCommon.h"
#include <ShellAPI.h>
#include <Ddeml.h>

using namespace std;

static HDDEDATA CALLBACK DDECallback(
	WORD     wType,
	WORD     wFmt,
	HCONV    hConv,
	HSZ      hsz1,
	HSZ      hsz2,
	HDDEDATA hData,
	DWORD    lData1,
	DWORD    lData2)
{
	return (HDDEDATA)0;
}

static bool OpenByExistingIE(const wstring &url)
{
	DWORD dwDDEID = 0;

	UINT err = DdeInitializeW(
		&dwDDEID,
		(PFNCALLBACK)MakeProcInstance((FARPROC)DDECallback, ghInstance),
		CBF_SKIP_ALLNOTIFICATIONS | APPCMD_CLIENTONLY, 0L);
	if (err != DMLERR_NO_ERROR)
		return false;

	HSZ hszService = DdeCreateStringHandleW(dwDDEID, L"IEXPLORE", CP_WINUNICODE);
	HSZ hszTopic = DdeCreateStringHandleW(dwDDEID, L"WWW_OpenURL", CP_WINUNICODE);
	HCONV hConv = DdeConnect(dwDDEID, hszService, hszTopic, NULL);
	DdeFreeStringHandle(dwDDEID, hszService);
	DdeFreeStringHandle(dwDDEID, hszTopic);

	if(!hConv)
		return false;

	CString cmd = url.c_str();
	HDDEDATA hDDEData = DdeClientTransaction(
		(LPBYTE)url.c_str(),
		static_cast<DWORD>(((url.size() + 1) * sizeof(wchar_t))),
		hConv,
		0,
		0,
		XTYP_EXECUTE,
		10000,
		NULL);
	if (hDDEData)
		DdeFreeDataHandle(hDDEData);
	DdeDisconnect(hConv);
	DdeUninitialize(dwDDEID);

	return true;
}

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
