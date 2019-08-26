#include <string>
#include <vector>
#include <atlbase.h>
#include <atlutil.h>
#include <ShellAPI.h>

typedef std::pair<std::wstring, std::wstring> MatchingPattern;
typedef std::vector<MatchingPattern> MatchingPatterns;

static void LoadDefaultBrowserName(std::wstring &browserName, bool systemWide = false)
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

static void LoadIEPath(std::wstring &path)
{
	LoadAppPath(path, _T("iexplore.exe"));
}

static void LoadFirefoxPath(std::wstring &path)
{
	LoadAppPath(path, _T("firefox.exe"));
}

static void LoadGoogleChromePath(std::wstring &path)
{
	LoadAppPath(path, _T("chrome.exe"));
}

static void LoadBrowserPath(std::wstring &path, const std::wstring &browserName)
{
	if (browserName == L"ie")
		LoadIEPath(path);
	if (browserName == L"firefox")
		LoadFirefoxPath(path);
	else if (browserName == L"chrome")
		LoadGoogleChromePath(path);
}

static void LoadDefaultBrowserNameAndPath(std::wstring &name, std::wstring &path)
{
	bool systemWide = true;
	::LoadDefaultBrowserName(name, systemWide);
	::LoadDefaultBrowserName(name);
	::LoadBrowserPath(path, name);
}

static void LoadMatchingPatterns(
	MatchingPatterns &patterns,
	const LPCTSTR type,
	bool systemWide = false)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector\\"));
	regKeyName += type;

	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);

	for (DWORD idx = 0; result == ERROR_SUCCESS; idx++) {
		TCHAR valueName[256];
		DWORD valueNameLen = 256;
		result = ::RegEnumValue(reg.m_hKey, idx, valueName, &valueNameLen, NULL, NULL, NULL, NULL);
		if (result != ERROR_SUCCESS)
			continue;
		TCHAR value[256];
		DWORD valueLen = 256;
		result = reg.QueryStringValue(valueName, value, &valueLen);
		patterns.push_back(MatchingPattern(valueName, value));
	}

	reg.Close();
}

static void LoadHostNamePatterns(MatchingPatterns &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("HostNamePatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("HostNamePatterns"));
}

static void LoadURLPatterns(MatchingPatterns &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("URLPatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("URLPatterns"));
}

static std::wstring GetBrowserNameToOpenURL(
	const std::wstring &url,
	const std::wstring &defaultBrowserName,
	const MatchingPatterns &hostNamePatterns,
	const MatchingPatterns &urlPatterns)
{
	static CComAutoCriticalSection symMatchSection;

	if (url.empty())
		return false;

	MatchingPatterns::const_iterator it;
	CUrl cURL;
	cURL.CrackUrl(url.c_str());
	LPCTSTR hostName = cURL.GetHostName();

	if (hostName && *hostName) {
		for (it = hostNamePatterns.begin(); it != hostNamePatterns.end(); it++) {
			symMatchSection.Lock();
			const std::wstring &hostNamePattern = it->first;
			BOOL matched = SymMatchStringW(cURL.GetHostName(), hostNamePattern.c_str(), FALSE);
			symMatchSection.Unlock();
			if (matched)
				return it->second;
		}
	}

	for (it = urlPatterns.begin(); it != urlPatterns.end(); it++) {
		symMatchSection.Lock();
		const std::wstring &urlPattern = it->first;
		BOOL matched = SymMatchStringW(url.c_str(), urlPattern.c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return it->second;
	}

	return defaultBrowserName;
}

bool OpenByModernBrowser(const std::wstring &browserName, const std::wstring &url)
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
