#include <string>
#include <vector>
#include <regex>
#include <atlbase.h>
#include <atlutil.h>
#include <ShellAPI.h>
#include <Shlobj.h>
#include <Ddeml.h>

typedef std::pair<std::wstring, std::wstring> SwitchingPattern;
typedef std::vector<SwitchingPattern> SwitchingPatterns;

void DebugLog(wchar_t *fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	wchar_t buf[1024];
	int nWritten = _vsnwprintf_s(buf, sizeof(buf) / sizeof(wchar_t), fmt, args);
	if (nWritten > 0)
		OutputDebugString(buf);
	va_end(args);
}

class Config {
public:
	Config()
		: m_debug(-1)
		, m_closeEmptyTab(-1)
		, m_onlyOnAnchorClick(-1)
		, m_useRegex(-1)
	{
	}
	virtual ~Config()
	{
	};

	virtual std::wstring getName()
	{
		return std::wstring();
	};

	virtual void dump()
	{
		if (m_debug <= 0)
			return;

		DebugLog(L"Config: %s", getName().c_str());
		DebugLog(L"  DefaultBrowser: %s", m_defaultBrowser.c_str());
		DebugLog(L"  SecondBrowser: %s", m_secondBrowser.c_str());
		DebugLog(L"  CloseEmptyTab: %d", m_closeEmptyTab);
		DebugLog(L"  OnlyOnAnchorClick: %d", m_onlyOnAnchorClick);
		DebugLog(L"  UseRegex: %d", m_useRegex);

		SwitchingPatterns::iterator it;

		DebugLog(L"  URLPatterns:");
		for (it = m_urlPatterns.begin(); it != m_urlPatterns.end(); it++) {
			DebugLog(L"    URL: %s Bowser: %s",
				it->first.c_str(), it->second.c_str());
		}

		DebugLog(L"  HostNamePatterns");
		for (it = m_hostNamePatterns.begin(); it != m_hostNamePatterns.end(); it++) {
			DebugLog(L"    Hostname: %s Bowser: %s",
				it->first.c_str(), it->second.c_str());
		}
	};

	void merge(std::vector<Config*> &configs)
	{
		std::vector<Config*>::iterator it;
		for (it = configs.begin(); it != configs.end(); it++) {
			Config *config = *it;
			if (config->m_debug >= 0)
				m_debug = config->m_debug;
			if (!config->m_defaultBrowser.empty())
				m_defaultBrowser = config->m_defaultBrowser;
			if (!config->m_secondBrowser.empty())
				m_secondBrowser = config->m_secondBrowser;
			if (config->m_closeEmptyTab >= 0)
				m_closeEmptyTab = config->m_closeEmptyTab;
			if (config->m_onlyOnAnchorClick >= 0)
				m_onlyOnAnchorClick = config->m_onlyOnAnchorClick;
			if (config->m_useRegex >= 0)
				m_useRegex = config->m_useRegex;

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

	void parseSwitchingPattern(SwitchingPattern &pattern, TCHAR *buf, DWORD nChars)
	{
		pattern.first.assign(buf, nChars);

		for (long i = nChars - 1; i >= 0; i--) {
			if (m_useRegex > 0) {
				if (buf[i] != '$')
					continue;
				if (i > 0 && buf[i - 1] == '\\') {
					// '\$' (escaped '$') should be ignored.
					// Since '\' is not allowed to use in URL, we don't consider the case
					// '\\\$' in the regex.
					continue;
				}
				pattern.first = pattern.first.substr(0, i + 1);
				pattern.second = buf + i + 1;
				return;
			} else {
				if (buf[i] != '|')
					continue;
				pattern.first = pattern.first.substr(0, i);
				pattern.second = buf + i + 1;
				return;
			}
		}
	};

	void LoadAll(HINSTANCE hInstance = nullptr);

public:
	int m_debug;
	std::wstring m_defaultBrowser;
	std::wstring m_secondBrowser;
	int m_closeEmptyTab;
	int m_onlyOnAnchorClick;
	int m_useRegex;
	SwitchingPatterns m_hostNamePatterns;
	SwitchingPatterns m_urlPatterns;
};

class DefaultConfig : public Config
{
public:
	DefaultConfig()
	{
		m_debug = 0;
		m_defaultBrowser = L"ie";
		m_closeEmptyTab = true;
		m_onlyOnAnchorClick = false;
		m_useRegex = false;
	}
	virtual ~DefaultConfig()
	{
	};

	virtual std::wstring getName()
	{
		return std::wstring(L"Default");
	}
};

class RegistryConfig : public Config
{
public:
	RegistryConfig(bool systemWide = false)
		: m_systemWide(systemWide)
	{
		LoadIntValue(m_debug, L"Debug", m_systemWide);

		LoadStringValue(m_defaultBrowser,
			L"DefaultBrowser", m_systemWide);
		LoadStringValue(m_secondBrowser,
			L"SecondBrowser", m_systemWide);
		LoadIntValue(m_closeEmptyTab, L"CloseEmptyTab", m_systemWide);
		LoadIntValue(m_onlyOnAnchorClick, L"OnlyOnAnchorClick", m_systemWide);
		LoadIntValue(m_useRegex, L"UseRegex", m_systemWide);
		LoadHostNamePatterns(m_hostNamePatterns);
		LoadURLPatterns(m_urlPatterns);

		dump();
	}
	virtual ~RegistryConfig()
	{
	}

	virtual std::wstring getName()
	{
		if (m_systemWide)
			return std::wstring(L"HKLM");
		else
			return std::wstring(L"HKCU");
	}

	static void LoadStringValue(
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

	static void LoadIntValue(
		int &value,
		const std::wstring &name,
		bool systemWide = false)
	{
		CRegKey reg;
		HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
		CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector"));
		LONG result = reg.Open(keyParent, regKeyName, KEY_READ);
		if (result == ERROR_SUCCESS) {
			DWORD v;
			result = reg.QueryDWORDValue(name.c_str(), v);
			if (result == ERROR_SUCCESS)
				value = v;
		}
		reg.Close();
	}

	void LoadSwitchingPatterns(
		SwitchingPatterns &patterns,
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
			if (result != ERROR_SUCCESS || valueLen < 1)
				continue;

			patterns.push_back(SwitchingPattern());
			parseSwitchingPattern(patterns.back(), value, valueLen);
		}

		reg.Close();
	}

	void LoadHostNamePatterns(SwitchingPatterns &patterns)
	{
		LoadSwitchingPatterns(patterns, _T("HostNamePatterns"), m_systemWide);
	}

	void LoadURLPatterns(SwitchingPatterns &patterns)
	{
		LoadSwitchingPatterns(patterns, _T("URLPatterns"), m_systemWide);
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
		, m_enableIncludeCache(false)
		, m_notFound(false)
	{
		if (m_path.empty() || !::PathFileExists(m_path.c_str())) {
			m_notFound = true;
			return;
		}

		GetIntValue(m_debug, L"Common", L"Debug");

		GetStringValue(m_defaultBrowser, L"Common", L"DefaultBrowser");
		GetStringValue(m_secondBrowser, L"Common", L"SecondBrowser");
		GetStringValue(m_includePath, L"Common", L"Include");
		GetIntValue(m_enableIncludeCache, L"Common", L"EnableIncludeCache");
		GetIntValue(m_closeEmptyTab, L"Common", L"CloseEmptyTab");
		GetIntValue(m_onlyOnAnchorClick, L"Common", L"OnlyOnAnchorClick");
		GetIntValue(m_useRegex, L"Common", L"UseRegex");
		LoadURLPatterns(m_urlPatterns);
		LoadHostNamePatterns(m_hostNamePatterns);

		dump();

		if (!m_includePath.empty() && !parent)
			includeINIFile(m_includePath);
	}
	virtual ~INIFileConfig()
	{
	};

	virtual void dump()
	{
		if (m_debug <= 0)
			return;
		Config::dump();

		DebugLog(L"  Include: %s", m_includePath.c_str());
		DebugLog(L"  EnableIncludeCache: %d", m_enableIncludeCache);
	}
	virtual std::wstring getName()
	{
		return m_path;
	}

	void includeINIFile(std::wstring &path)
	{
		INIFileConfig child(path, this);
		if (m_enableIncludeCache) {
			if (child.isSucceededToLoad())
				child.WriteCache();
			else
				child.ReadCache();
		}

		std::vector<Config*> configs;
		configs.push_back(&child);
		merge(configs);
	}

	bool isSucceededToLoad()
	{
		return !m_notFound;
	}

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

	void LoadHostNamePatterns(SwitchingPatterns &hostNamePatterns)
	{
		LoadSwitchingPatterns(hostNamePatterns, L"HostNamePatterns");
	}

	void LoadURLPatterns(SwitchingPatterns &urlPatterns)
	{
		LoadSwitchingPatterns(urlPatterns, L"URLPatterns");
	}

	void LoadSwitchingPatterns(SwitchingPatterns &patterns, LPCTSTR type)
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

			patterns.push_back(SwitchingPattern());
			parseSwitchingPattern(patterns.back(), buf, nWrittenChars);
		}
	}

	void ReadCache()
	{
		std::wstring path = GetCachePath(m_path);
		if (path.empty())
			return;
		INIFileConfig cache(path, this);
		std::vector<Config*> configs;
		configs.push_back(&cache);
		merge(configs);
	}

	void WriteCache()
	{
		std::wstring folderPath = GetCacheFolderPath();
		std::wstring cachePath = GetCachePath(m_path);

		if (cachePath.empty())
			return;

		unsigned int pos = GetDataFolderPath().size();
		do
		{
			pos = folderPath.find_first_of(L"\\", pos + 1);
			std::wstring path;
			if (pos == std::string::npos)
				path = folderPath;
			else
				path = cachePath.substr(0, pos).c_str();
			::CreateDirectory(path.c_str(), NULL);
		} while (pos != std::string::npos);

		if (!::PathFileExists(folderPath.c_str()))
			return;

		::CopyFile(m_path.c_str(), cachePath.c_str(), FALSE);
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

	static std::wstring GetDataFolderPath(void)
	{
		TCHAR buf[MAX_PATH];
		DWORD bufSize = sizeof(buf) / sizeof(TCHAR);
		BOOL succeeded = ::SHGetSpecialFolderPath(NULL, buf, CSIDL_APPDATA, FALSE);
		if (!succeeded)
			return std::wstring();
		std::wstring path(buf);
		return path;
	}

	static std::wstring GetAppDataFolderPath(void)
	{
		std::wstring path(GetDataFolderPath());
		if (path.empty())
			return path;
		path += std::wstring(L"\\ClearCode\\BrowserSelector");
		return path;
	}

	static std::wstring GetUserConfigPath(void)
	{
		std::wstring path(GetAppDataFolderPath());
		if (path.empty())
			return path;
		path += std::wstring(L"\\BrowserSelector.ini");
		return path;
	}

	static std::wstring GetTempPath(void)
	{
		LPWSTR folderPath;
		HRESULT result = SHGetKnownFolderPath(
			FOLDERID_LocalAppDataLow, 0, NULL, &folderPath);
		if (FAILED(result))
			return std::wstring();
		std::wstring path(folderPath);
		CoTaskMemFree(folderPath);
		return path;
	}


	static std::wstring GetCacheFolderPath(void)
	{
		std::wstring path(GetTempPath());
		if (path.empty())
			return path;
		path += std::wstring(L"\\ClearCode\\BrowserSelector");
		return path;
	}

	static std::wstring GetCachePath(const std::wstring &srcPath)
	{
		std::wstring path(GetCacheFolderPath());
		if (path.empty())
			return path;

		path += std::wstring(L"\\");

		std::wstring filename(srcPath);
		for (unsigned int i = 0; i < filename.size(); i++) {
			int ch = filename[i];
			if (ch == '\\' || ch == ':')
				filename[i] = '_';
		}
		path += filename;
		return path;
	}

public:
	std::wstring m_path;
	std::wstring m_includePath;
	int m_enableIncludeCache;
	bool m_notFound;
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

	dump();
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
	else if (browserName == L"firefox")
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
	const Config &config,
	const std::wstring *name = nullptr)
{
	if (name && isValidBrowserName(*name)) {
		return *name;
	} else if (name && name->empty() && isValidBrowserName(config.m_secondBrowser)) {
		if (config.m_debug > 0)
			DebugLog(L"Use second browser: %s", config.m_secondBrowser.c_str());
		return config.m_secondBrowser;
	} else if (isValidBrowserName(config.m_defaultBrowser)) {
		if (config.m_debug > 0)
			DebugLog(L"Use default browser: %s", config.m_defaultBrowser.c_str());
		return config.m_defaultBrowser;
	} else {
		if (config.m_debug > 0)
			DebugLog(L"Fall back to IE");
		return std::wstring(L"ie");
	}
}

static bool matchSimpleWildCard(const std::wstring &url, const std::wstring &pattern)
{
	static CComAutoCriticalSection symMatchSection;
	symMatchSection.Lock();
	BOOL matched = SymMatchStringW(url.c_str(), pattern.c_str(), FALSE);
	symMatchSection.Unlock();
	return matched ? true : false;
}

static bool matchRegex(const std::wstring &url, const std::wstring &pattern, const Config &config)
{
	std::string urlASCII, patternASCII;
	for (DWORD i = 0; i < url.size(); i++) {
		int ch = url[i];
		urlASCII += ch;
	}
	for (DWORD i = 0; i < pattern.size(); i++) {
		int ch = pattern[i];
		patternASCII += ch;
	}

	try {
		std::regex re(patternASCII);
		std::smatch match;
		return std::regex_match(urlASCII, match, re);
	} catch (std::regex_error &e) {
		if (config.m_debug > 0)
			DebugLog(
				L"Failed to compile the regex! pattern: %s, message: %s",
				pattern.c_str(), e.what());
		return false;
	}
}

static bool matchURL(const std::wstring &url, const std::wstring &pattern, const Config &config)
{
	if (config.m_useRegex > 0)
		return matchRegex(url, pattern, config);
	else
		return matchSimpleWildCard(url, pattern);
}

static std::wstring GetBrowserNameToOpenURL(
	const std::wstring &url,
	const Config &config)
{
	if (url.empty())
		return ensureValidBrowserName(config);

	SwitchingPatterns::const_iterator it;

	for (it = config.m_urlPatterns.begin(); it != config.m_urlPatterns.end(); it++) {
		const std::wstring &urlPattern = it->first;
		bool matched = matchURL(url, urlPattern, config);
		if (!matched)
			continue;
		if (config.m_debug > 0)
			DebugLog(L"Matched URL pattern: %s Browser: %s",
				it->first.c_str(), it->second.c_str());
		return ensureValidBrowserName(config, &it->second);
	}

	CUrl cURL;
	cURL.CrackUrl(url.c_str());
	LPCTSTR hostName = cURL.GetHostName();

	if (hostName && *hostName) {
		for (it = config.m_hostNamePatterns.begin(); it != config.m_hostNamePatterns.end(); it++) {
			const std::wstring &hostNamePattern = it->first;
			bool matched = matchURL(hostName, hostNamePattern, config);
			if (!matched)
				continue;
			if (config.m_debug > 0)
				DebugLog(L"Matched hostname pattern: %s Browser: %s",
					it->first.c_str(), it->second.c_str());
			return ensureValidBrowserName(config, &it->second);
		}
	}

	DebugLog(L"Unmatched: %s", url.c_str());

	return ensureValidBrowserName(config);
}

bool OpenByModernBrowser(
	const std::wstring &browserName,
	const std::wstring &url,
	bool bypassElevationPolicy = false)
{
	HINSTANCE hInstance = 0;
	if (bypassElevationPolicy) {
		// Avoid showing the elevation warning dialog of IE. BrowserSelector.exe is
		// registered as Policy == 3 (don't show the dialog) by the installer.
		hInstance = ::ShellExecute(
			NULL, // HWND
			_T("open"), _T("BrowserSelector.exe"), url.c_str(),
			NULL, // Directory
			SW_SHOW);
	} else {
		if (browserName == L"firefox") {
			hInstance = ::ShellExecute(NULL, _T("open"), _T("firefox.exe"), url.c_str(), NULL, SW_SHOW);
		} else if (browserName == L"chrome") {
			hInstance = ::ShellExecute(NULL, _T("open"), _T("chrome.exe"), url.c_str(), NULL, SW_SHOW);
		}
	}
	return (reinterpret_cast<int>(hInstance) > 32);
}

/*
 * OpenByExistingIE
 */
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

static bool OpenByExistingIE(const std::wstring &url)
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
