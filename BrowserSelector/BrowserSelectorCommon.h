#include <string>
#include <vector>
#include <atlbase.h>
#include <atlutil.h>
#include <ShellAPI.h>

static void LoadSecondBrowserName(std::wstring &browserName, bool systemWide = false)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector"));
	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);
	if (result == ERROR_SUCCESS) {
		TCHAR value[256];
		ULONG valueSize = sizeof(value);
		result = reg.QueryStringValue(L"DefaultBrowser", value, &valueSize);
		if (result == ERROR_SUCCESS)
			browserName = value;
	}
	reg.Close();
}

static void LoadAppPath(std::wstring &wpath, const LPCTSTR exeName)
{
	CRegKey reg;
	CString regKeyName(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"));
	regKeyName += exeName;

	LONG result = reg.Open(HKEY_LOCAL_MACHINE, regKeyName, KEY_READ);
	if (result != ERROR_SUCCESS)
		return;

	TCHAR path[MAX_PATH];
	ULONG pathSize = MAX_PATH;
	result = reg.QueryStringValue(NULL, path, &pathSize);
	if (result == ERROR_SUCCESS)
		wpath = path;

	reg.Close();
}

static void LoadFirefoxPath(std::wstring &path)
{
	LoadAppPath(path, _T("firefox.exe"));
}

static void LoadGoogleChromePath(std::wstring &path)
{
	LoadAppPath(path, _T("chrome.exe"));
}

static void LoadSecondBrowserPath(const std::wstring &browserName, std::wstring &path)
{
	if (browserName == L"firefox")
		LoadFirefoxPath(path);
	else if (browserName == L"chrome")
		LoadGoogleChromePath(path);
}

static void LoadSecondBrowserNameAndPath(std::wstring &name, std::wstring &path)
{
	bool systemWide = true;
	::LoadSecondBrowserName(name, systemWide);
	::LoadSecondBrowserName(name);
	::LoadSecondBrowserPath(name, path);
}

static void LoadMatchingPatterns(
	std::vector<std::wstring> &patterns,
	const LPCTSTR type,
	bool systemWide = false)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector\\"));
	regKeyName += type;

	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);

	for (DWORD idx = 0; result == ERROR_SUCCESS; idx++) {
		TCHAR value[256];
		DWORD valueLen = 256;
		result = ::RegEnumValue(reg.m_hKey, idx,value, &valueLen, NULL, NULL, NULL, NULL);
		if (result != ERROR_SUCCESS)
			continue;
		patterns.push_back(value);
	}

	reg.Close();
}

static void LoadHostNamePatterns(std::vector<std::wstring> &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("IntranetHostNamePatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("IntranetHostNamePatterns"));
}

static void LoadURLPatterns(std::vector<std::wstring> &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("IntranetURLPatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("IntranetURLPatterns"));
}

static bool IsIntranetURL(
	const std::wstring &url,
	const std::vector<std::wstring> &hostNamePatterns,
	const std::vector<std::wstring> &urlPatterns)
{
	static CComAutoCriticalSection symMatchSection;

	if (url.empty())
		return false;

	std::vector<std::wstring>::const_iterator it;
	CUrl cURL;
	cURL.CrackUrl(url.c_str());
	LPCTSTR hostName = cURL.GetHostName();

	if (hostName && *hostName) {
		for (it = hostNamePatterns.begin(); it != hostNamePatterns.end(); it++) {
			symMatchSection.Lock();
			BOOL matched = SymMatchStringW(cURL.GetHostName(), it->c_str(), FALSE);
			symMatchSection.Unlock();
			if (matched)
				return true;
		}
	}

	for (it = urlPatterns.begin(); it != urlPatterns.end(); it++) {
		symMatchSection.Lock();
		BOOL matched = SymMatchStringW(url.c_str(), it->c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return true;
	}

	return false;
}

bool OpenBySecondBrowser(const std::wstring &browserName, const std::wstring &url)
{
	HINSTANCE hInstance = 0;
	if (browserName == L"firefox") {
		hInstance = ::ShellExecute(NULL, _T("open"), _T("firefox.exe"), url.c_str(), NULL, SW_SHOW);
		return (reinterpret_cast<int>(hInstance) > 32);
	} else if (browserName == L"chrome") {
		hInstance = ::ShellExecute(NULL, _T("open"), _T("chrome.exe"), url.c_str(), NULL, SW_SHOW);
		return (reinterpret_cast<int>(hInstance) > 32);
	}
	return false;
}
