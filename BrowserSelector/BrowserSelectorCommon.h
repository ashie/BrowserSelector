#include <string>
#include <vector>
#include <atlbase.h>
#include <atlutil.h>
#include <ShellAPI.h>
#include <Shlobj.h>

typedef std::pair<std::wstring, std::wstring> MatchingPattern;
typedef std::vector<MatchingPattern> MatchingPatterns;

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

class Config {
public:
	Config()
		: m_closeEmptyTab(-1)
	{
	}
	virtual ~Config()
	{
	};

	void merge(std::vector<Config*> &configs)
	{
		std::vector<Config*>::iterator it;
		for (it = configs.begin(); it != configs.end(); it++) {
			Config *config = *it;
			if (!config->m_defaultBrowser.empty())
				m_defaultBrowser = config->m_defaultBrowser;
			if (config->m_closeEmptyTab >= 0)
				m_closeEmptyTab = config->m_closeEmptyTab;

			m_hostNamePatterns.insert(
				m_hostNamePatterns.begin(),
				config->m_hostNamePatterns.begin(),
				config->m_hostNamePatterns.end());
			m_urlPatterns.insert(
				m_urlPatterns.begin(),
				config->m_urlPatterns.begin(),
				config->m_urlPatterns.end());
		}
	}

	void LoadAll(HINSTANCE hInstance = nullptr);

public:
	std::wstring m_defaultBrowser;
	int m_closeEmptyTab;
	MatchingPatterns m_hostNamePatterns;
	MatchingPatterns m_urlPatterns;
};

class DefaultConfig : public Config
{
public:
	DefaultConfig()
	{
		m_defaultBrowser = L"ie";
		m_closeEmptyTab = true;
	}
	virtual ~DefaultConfig()
	{
	};
};

class RegistryConfig : public Config
{
public:
	RegistryConfig(bool systemWide = false)
		: m_systemWide(systemWide)
	{
		LoadDefaultBrowserName(m_defaultBrowser);
		LoadHostNamePatterns(m_hostNamePatterns);
		LoadURLPatterns(m_urlPatterns);
	}
	virtual ~RegistryConfig()
	{
	}

	void LoadDefaultBrowserName(std::wstring &name)
	{
		::LoadStringRegValue(name, L"DefaultBrowser", m_systemWide);
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

	void LoadHostNamePatterns(MatchingPatterns &patterns)
	{
		LoadMatchingPatterns(patterns, _T("HostNamePatterns"), m_systemWide);
	}

	void LoadURLPatterns(MatchingPatterns &patterns)
	{
		LoadMatchingPatterns(patterns, _T("URLPatterns"), m_systemWide);
	}

public:
	bool m_systemWide;
};

class INIFileConfig : public Config
{
public:
	INIFileConfig(const std::wstring &path, INIFileConfig *parent = nullptr)
		: m_path(path)
		, m_parent(parent)
	{
		GetStringValue(m_defaultBrowser, L"Common", L"DefaultBrowser");
		GetStringValue(m_includePath, L"Common", L"Include");
		GetIntValue(m_closeEmptyTab, L"Common", L"CloseEmptyTab");
		LoadURLPatterns(m_urlPatterns);
		LoadHostNamePatterns(m_hostNamePatterns);

		if (!m_includePath.empty() && !parent) {
			INIFileConfig child(m_includePath, this);
			std::vector<Config*> configs;
			configs.push_back(&child);
			merge(configs);
		}
	}
	virtual ~INIFileConfig()
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
	static std::wstring GetSystemConfigPath(HINSTANCE hInstance = nullptr)
	{
		TCHAR buf[MAX_PATH];
		DWORD bufSize = sizeof(buf) / sizeof(TCHAR);
		DWORD ret = ::GetModuleFileName(hInstance, buf, bufSize);
		if (ret < 1)
			return std::wstring();
		TCHAR *lastSep = _tcsrchr(buf, '\\');
		if (lastSep)
			*(++lastSep) = '\0';
		std::wstring path(buf);
		path += std::wstring(L"BrowserSelector.ini");
		return path;
	}

	static std::wstring GetUserConfigPath(void)
	{
		TCHAR buf[MAX_PATH];
		DWORD bufSize = sizeof(buf) / sizeof(TCHAR);
		BOOL succeeded = ::SHGetSpecialFolderPath(NULL, buf, CSIDL_APPDATA, FALSE);
		if (!succeeded)
			return std::wstring();
		std::wstring path(buf);
		path += std::wstring(L"\\ClearCode\\BrowserSelector\\BrowserSelector.ini");
		return path;
	}

public:
	std::wstring m_path;
	std::wstring m_includePath;
	INIFileConfig *m_parent;
};

void Config::LoadAll(HINSTANCE hInstance)
{
	bool systemWide = true;
	DefaultConfig defaultConfig;
	RegistryConfig systemConfig(systemWide);
	RegistryConfig userConfig;
	INIFileConfig systemINIFileConfig(INIFileConfig::GetSystemConfigPath(hInstance));
	INIFileConfig userINIFileConfig(INIFileConfig::GetUserConfigPath());

	std::vector<Config*> configs;
	configs.push_back(&defaultConfig);
	configs.push_back(&systemConfig);
	configs.push_back(&systemINIFileConfig);
	configs.push_back(&userConfig);
	configs.push_back(&userINIFileConfig);

	merge(configs);
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
	const Config &config)
{
	static CComAutoCriticalSection symMatchSection;

	if (url.empty())
		return ensureValidBrowserName(config.m_defaultBrowser);

	MatchingPatterns::const_iterator it;
	CUrl cURL;
	cURL.CrackUrl(url.c_str());
	LPCTSTR hostName = cURL.GetHostName();

	if (hostName && *hostName) {
		for (it = config.m_hostNamePatterns.begin(); it != config.m_hostNamePatterns.end(); it++) {
			symMatchSection.Lock();
			const std::wstring &hostNamePattern = it->first;
			BOOL matched = SymMatchStringW(cURL.GetHostName(), hostNamePattern.c_str(), FALSE);
			symMatchSection.Unlock();
			if (matched)
				return ensureValidBrowserName(config.m_defaultBrowser, it->second);
		}
	}

	for (it = config.m_urlPatterns.begin(); it != config.m_urlPatterns.end(); it++) {
		symMatchSection.Lock();
		const std::wstring &urlPattern = it->first;
		BOOL matched = SymMatchStringW(url.c_str(), urlPattern.c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return ensureValidBrowserName(config.m_defaultBrowser, it->second);
	}

	return ensureValidBrowserName(config.m_defaultBrowser);
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
