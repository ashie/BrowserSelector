#include <string>
#include <vector>
#include <atlbase.h>
#include <atlutil.h>
#include <ShellAPI.h>

typedef std::pair<std::wstring, std::wstring> MatchingPattern;
typedef std::vector<MatchingPattern> MatchingPatterns;

class INIFile {
public:
	INIFile(const std::wstring &path)
		: m_path(path)
		, m_closeEmptyTab(-1)
	{
		GetStringValue(m_defaultBrowser, L"Common", L"DefaultBrowser");
		GetIntValue(m_closeEmptyTab, L"Common", L"CloseEmptyTab");
		LoadURLPatterns(m_urlPatterns);
		LoadHostNamePatterns(m_hostNamePatterns);
	}
	~INIFile()
	{
	};

	void GetIntValue(int &value, const std::wstring &section, const std::wstring &key)
	{
		value = GetPrivateProfileInt(
			section.c_str(), key.c_str(), value, m_path.c_str());
	}

	void GetStringValue(std::wstring &value, const std::wstring &section, const std::wstring &key)
	{
		TCHAR buf[256];
		DWORD size = sizeof(buf) / sizeof(TCHAR);
		DWORD nWrittenChars = GetPrivateProfileString(
			section.c_str(), key.c_str(), value.c_str(), buf, size, m_path.c_str());
		if (nWrittenChars > 0)
			value = buf;
	}

	void GetKeys(std::vector<std::wstring> &keys, const std::wstring &sectionName)
	{
		TCHAR buf[4096];
		DWORD size = sizeof(buf) / sizeof(TCHAR);
		DWORD nWrittenChars = GetPrivateProfileString(
			sectionName.c_str(), NULL, NULL, buf, size, m_path.c_str());
		TCHAR *key = buf;
		for (DWORD i = 0; i < nWrittenChars; i++) {
			if (!buf[i]) {
				if (*key)
					keys.push_back(key);
				key = buf + i + 1;
			}
		}
	}

	void LoadHostNamePatterns(MatchingPatterns &hostNamePatterns)
	{
		LoadMatchingPatterns(hostNamePatterns, L"HostNamePatterns");
	}

	void LoadURLPatterns(MatchingPatterns &urlPatterns)
	{
		LoadMatchingPatterns(urlPatterns, L"URLPatterns");
	}

	void LoadMatchingPatterns(MatchingPatterns &patterns, LPCTSTR type)
	{
		std::vector<std::wstring> keys;
		GetKeys(keys, type);
		std::vector<std::wstring>:: iterator it;
		for (it = keys.begin(); it != keys.end(); it++) {
			TCHAR buf[1024];
			DWORD size = sizeof(buf) / sizeof(TCHAR);
			DWORD nWrittenChars = GetPrivateProfileString(
				type, it->c_str(), NULL, buf, size, m_path.c_str());
			if (nWrittenChars < 1)
				continue;

			TCHAR *browserName = L"";
			for (DWORD i = 0; i < nWrittenChars; i++) {
				if (buf[i] == '|') {
					buf[i] = '\0';
					browserName = buf + i + 1;
					break;
				}
			}

			patterns.push_back(MatchingPattern(buf, browserName));
		}
	}

public:
	std::wstring m_path;
	std::wstring m_defaultBrowser;
	int m_closeEmptyTab;
	MatchingPatterns m_hostNamePatterns;
	MatchingPatterns m_urlPatterns;
};

static void LoadStringRegValue(
	std::wstring &value,
	const std::wstring &name,
	bool systemWide = false)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector"));
	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);
	if (result == ERROR_SUCCESS) {
		TCHAR regValue[256];
		ULONG regValueSize = sizeof(regValue) / sizeof(TCHAR);
		result = reg.QueryStringValue(name.c_str(), regValue, &regValueSize);
		if (result == ERROR_SUCCESS)
			value = regValue;
	}
	reg.Close();
}

static void LoadDefaultBrowserName(std::wstring &name)
{
	bool systemWide = true;
	::LoadStringRegValue(name, L"DefaultBrowser", systemWide);
	::LoadStringRegValue(name, L"DefaultBrowser");
}

static void LoadMatchingPatterns(
	MatchingPatterns &patterns,
	LPCTSTR type,
	bool systemWide = false)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector\\"));
	regKeyName += type;

	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);

	for (DWORD idx = 0; result == ERROR_SUCCESS; idx++) {
		TCHAR valueName[1024];
		DWORD valueNameLen = sizeof(valueName) / sizeof(TCHAR);
		result = ::RegEnumValue(reg.m_hKey, idx, valueName, &valueNameLen, NULL, NULL, NULL, NULL);
		if (result != ERROR_SUCCESS)
			continue;
		TCHAR value[1024];
		DWORD valueLen = sizeof(value) / sizeof(TCHAR);
		result = reg.QueryStringValue(valueName, value, &valueLen);
		patterns.push_back(MatchingPattern(valueName, value));
	}

	reg.Close();
}

static void LoadHostNamePatterns(MatchingPatterns &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("HostNamePatterns"));
	LoadMatchingPatterns(patterns, _T("HostNamePatterns"), systemWide);
}

static void LoadURLPatterns(MatchingPatterns &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("URLPatterns"));
	LoadMatchingPatterns(patterns, _T("URLPatterns"), systemWide);
}

static void LoadAppPath(std::wstring &wpath, LPCTSTR exeName)
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

static void LoadBrowserPath(std::wstring &path, const std::wstring &browserName)
{
	if (browserName == L"ie")
		LoadAppPath(path, _T("iexplore.exe"));
	if (browserName == L"firefox")
		LoadAppPath(path, _T("firefox.exe"));
	else if (browserName == L"chrome")
		LoadAppPath(path, _T("chrome.exe"));
}

static bool isInSystemPath(const std::wstring &browserName)
{
	std::wstring path;
	LoadBrowserPath(path, browserName);
	return !path.empty();
}

static bool isValidBrowserName(const std::wstring &browserName)
{
	if (browserName.empty())
		return false;
	return isInSystemPath(browserName);
}

static std::wstring ensureValidBrowserName(
	const std::wstring &defaultName,
	const std::wstring &name = std::wstring(L""))
{
	if (isValidBrowserName(name))
		return name;
	else if (isValidBrowserName(defaultName))
		return defaultName;
	else
		return std::wstring(L"ie");
}

static std::wstring GetBrowserNameToOpenURL(
	const std::wstring &url,
	const std::wstring &defaultBrowserName,
	const MatchingPatterns &hostNamePatterns,
	const MatchingPatterns &urlPatterns)
{
	static CComAutoCriticalSection symMatchSection;

	if (url.empty())
		return ensureValidBrowserName(defaultBrowserName);

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
				return ensureValidBrowserName(defaultBrowserName, it->second);
		}
	}

	for (it = urlPatterns.begin(); it != urlPatterns.end(); it++) {
		symMatchSection.Lock();
		const std::wstring &urlPattern = it->first;
		BOOL matched = SymMatchStringW(url.c_str(), urlPattern.c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return ensureValidBrowserName(defaultBrowserName, it->second);
	}

	return ensureValidBrowserName(defaultBrowserName);
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
