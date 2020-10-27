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

static void DebugLogV(wchar_t* fmt, va_list args)
{
	wchar_t buf[1024];
	size_t len = sizeof(buf) / sizeof(wchar_t);
	int nWritten = _vsnwprintf_s(buf, len, _TRUNCATE, fmt, args);
	if (nWritten > 0)
		OutputDebugString(buf);
}

static void DebugLog(wchar_t *fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	DebugLogV(fmt, args);
	va_end(args);
}

/*
 * Convert wstring into JSON reporesentation (i.e. aiu\n -> "aiu\\n")
 *
 * The escape list below is retrieved from CPython's implementation.
 * https://github.com/python/cpython/blob/3.7/Lib/json/encoder.py#L21
 */
static std::wstring ToJsonString(std::wstring src, std::wstring dest)
{
	const wchar_t *ptr = src.c_str();
	dest = L'"';
	while (*ptr) {
		switch (*ptr) {
			case L'\\': dest += L"\\\\"; break;
			case L'\"': dest += L"\\\""; break;
			case L'\b': dest += L"\\b"; break;
			case L'\f': dest += L"\\f"; break;
			case L'\n': dest += L"\\n"; break;
			case L'\r': dest += L"\\r"; break;
			case L'\t': dest += L"\\t"; break;
			default: dest += *ptr;
		}
		ptr++;
	}
	dest += L'"';
	return dest;
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

		DebugLog(L"Config: %ls", getName().c_str());
		DebugLog(L"  DefaultBrowser: %ls", m_defaultBrowser.c_str());
		DebugLog(L"  SecondBrowser: %ls", m_secondBrowser.c_str());
		DebugLog(L"  FirefoxCommand: %ls", m_firefoxCommand.c_str());
		DebugLog(L"  CloseEmptyTab: %d", m_closeEmptyTab);
		DebugLog(L"  OnlyOnAnchorClick: %d", m_onlyOnAnchorClick);
		DebugLog(L"  UseRegex: %d", m_useRegex);

		SwitchingPatterns::iterator it;

		DebugLog(L"  URLPatterns:");
		for (it = m_urlPatterns.begin(); it != m_urlPatterns.end(); it++) {
			DebugLog(L"    URL: %ls Browser: %ls",
				it->first.c_str(), it->second.c_str());
		}

		DebugLog(L"  HostNamePatterns");
		for (it = m_hostNamePatterns.begin(); it != m_hostNamePatterns.end(); it++) {
			DebugLog(L"    Hostname: %ls Browser: %ls",
				it->first.c_str(), it->second.c_str());
		}

		DebugLog(L"  ZonePatterns");
		for (it = m_zonePatterns.begin(); it != m_zonePatterns.end(); it++) {
			DebugLog(L"    Zone: %ls Browser: %ls",
				it->first.c_str(), it->second.c_str());
		}

	};

	virtual void dumpAsJson(std::wstring &buf) const
	{
		wchar_t tmp[10];
		const size_t tmpLen = sizeof(tmp) / sizeof(wchar_t);
		std::wstring strbuf;
		SwitchingPatterns::const_iterator it;

		buf = L"{";

		buf += L"\"DefaultBrowser\":\"";
		buf += m_defaultBrowser;
		buf += L"\",";

		buf += L"\"SecondBrowser\":\"";
		buf += m_secondBrowser;
		buf += L"\",";

		buf += L"\"FirefoxCommand\":";
		buf += ToJsonString(m_firefoxCommand, strbuf);
		buf += L",";

		buf += L"\"CloseEmptyTab\":";
		buf += _itow_s(m_closeEmptyTab, tmp, tmpLen, 10);
		buf += L",";

		buf += L"\"OnlyOnAnchorClick\":";
		buf += _itow_s(m_onlyOnAnchorClick, tmp, tmpLen, 10);
		buf += L",";

		buf += L"\"UseRegex\":";
		buf += _itow_s(m_useRegex, tmp, tmpLen, 10);
		buf += L",";

		buf += L"\"URLPatterns\":[";
		for (it = m_urlPatterns.begin(); it != m_urlPatterns.end(); it++) {
			if (it != m_urlPatterns.begin())
				buf += L",";
			buf += L"[\"";
			buf += it->first;
			buf += L"\",\"";
			buf += it->second;
			buf += L"\"]";
		}
		buf += L"],";

		buf += L"\"HostNamePatterns\":[";
		for (it = m_hostNamePatterns.begin(); it != m_hostNamePatterns.end(); it++) {
			if (it != m_hostNamePatterns.begin())
				buf += L",";
			buf += L"[\"";
			buf += it->first;
			buf += L"\",\"";
			buf += it->second;
			buf += L"\"]";
		}
		buf += L"],";

		buf += L"\"ZonePatterns\":[";
		for (it = m_zonePatterns.begin(); it != m_zonePatterns.end(); it++) {
			if (it != m_zonePatterns.begin())
				buf += L",";
			buf += L"[\"";
			buf += it->first;
			buf += L"\",\"";
			buf += it->second;
			buf += L"\"]";
		}

		buf += L"]}";
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
			if (!config->m_firefoxCommand.empty())
				m_firefoxCommand = config->m_firefoxCommand;
			if (config->m_closeEmptyTab >= 0)
				m_closeEmptyTab = config->m_closeEmptyTab;
			if (config->m_onlyOnAnchorClick >= 0)
				m_onlyOnAnchorClick = config->m_onlyOnAnchorClick;
			if (config->m_useRegex >= 0)
				m_useRegex = config->m_useRegex;

			m_zonePatterns.insert(
				m_zonePatterns.begin(),
				config->m_zonePatterns.begin(),
				config->m_zonePatterns.end());
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
	std::wstring m_firefoxCommand;
	int m_closeEmptyTab;
	int m_onlyOnAnchorClick;
	int m_useRegex;
	SwitchingPatterns m_zonePatterns;
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

class INIFileConfig : public Config
{
public:
	INIFileConfig(const std::wstring &path, Config *parent = nullptr)
		: m_path(path)
		, m_parent(parent)
		, m_enableIncludeCache(false)
	{
		if (m_path.empty() || !::PathFileExists(m_path.c_str()))
			return;

		GetIntValue(m_debug, L"Common", L"Debug");

		GetStringValue(m_defaultBrowser, L"Common", L"DefaultBrowser");
		GetStringValue(m_secondBrowser, L"Common", L"SecondBrowser");
		GetStringValue(m_firefoxCommand, L"Common", L"FirefoxCommand");
		GetStringValue(m_includePath, L"Common", L"Include");
		GetIntValue(m_enableIncludeCache, L"Common", L"EnableIncludeCache");
		GetIntValue(m_closeEmptyTab, L"Common", L"CloseEmptyTab");
		GetIntValue(m_onlyOnAnchorClick, L"Common", L"OnlyOnAnchorClick");
		GetIntValue(m_useRegex, L"Common", L"UseRegex");
		LoadURLPatterns(m_urlPatterns);
		LoadHostNamePatterns(m_hostNamePatterns);
		LoadZonePatterns(m_zonePatterns);

		dump();

		if (!m_includePath.empty() && !parent)
			Include(*this, m_includePath, m_enableIncludeCache, m_debug > 0);
	}
	virtual ~INIFileConfig()
	{
	};

	virtual void dump()
	{
		if (m_debug <= 0)
			return;
		Config::dump();

		DebugLog(L"  Include: %ls", m_includePath.c_str());
		DebugLog(L"  EnableIncludeCache: %d", m_enableIncludeCache);
	}

	virtual std::wstring getName()
	{
		return m_path;
	}

	static void MergeINIFile(Config& parent, std::wstring& path)
	{
		INIFileConfig child(path, &parent);
		std::vector<Config*> configs;
		configs.push_back(&child);
		parent.merge(configs);
	}

	static bool GetFileModifiedTime(const std::wstring &path, LPFILETIME time)
	{
		HANDLE hFile = CreateFile(
			path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);

		if (!hFile)
			return false;

		bool ret = GetFileTime(hFile, nullptr, nullptr, time);

		CloseHandle(hFile);

		return ret;
	}

	static bool IsCacheUpdated(const std::wstring &path, const std::wstring &cachePath)
	{
		FILETIME ft1, ft2;
		if (!GetFileModifiedTime(path, &ft1))
			return false;
		if (!GetFileModifiedTime(cachePath, &ft2))
			return false;

		LONG ret = CompareFileTime(&ft1, &ft2);
		return (ret <= 0);
	}

	static void Include(Config &parent, const std::wstring &path, bool useCache = false, bool debug = false)
	{
		std::wstring cachePath = GetCachePath(path);
		std::wstring tmpPath;

		if (debug)
			DebugLog(L"Try to include %ls", path.c_str());

		if (useCache && !cachePath.empty() && IsCacheUpdated(path, cachePath)) {
			if (debug) {
				DebugLog(L"Cache file for %ls is updated, attempt to load from it.", path.c_str());
				DebugLog(L"Cache path: %ls", cachePath.c_str());
			}
			MergeINIFile(parent, cachePath);
			return;
		}

		bool succeeded = CopyToTempFile(path, tmpPath);
		if (!succeeded) {
			if (debug)
				DebugLog(L"Failed to load external INI file!: %ls", path.c_str());

			// Fallback to the local cache file
			if (useCache && !cachePath.empty())
				MergeINIFile(parent, cachePath);

			// Ensure to clean the tmp file
			DeleteFile(tmpPath.c_str());

			return;
		}

		MergeINIFile(parent, tmpPath);
		if (useCache) {
			succeeded = MoveFileEx(tmpPath.c_str(), cachePath.c_str(), MOVEFILE_REPLACE_EXISTING);
			if (!succeeded) {
				if (debug)
					DebugLog(L"Failed to write INI file cache!: %ls", cachePath.c_str());
				// Ensure to clean the tmp file
				DeleteFile(tmpPath.c_str());
			}
		} else {
			succeeded = DeleteFile(tmpPath.c_str());
			if (!succeeded)
				if (debug)
					DebugLog(L"Failed to delete tmp INI file!: %ls", tmpPath.c_str());
		}

		if (succeeded)
			if (debug)
				DebugLog(L"Succeeded to include %ls", path.c_str());
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

	void LoadZonePatterns(SwitchingPatterns &zonePatterns)
	{
		LoadSwitchingPatterns(zonePatterns, L"ZonePatterns");
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

public:
	static bool CopyToTempFile(const std::wstring& srcPath, std::wstring &tmpPath)
	{
		std::wstring folderPath = GetCacheFolderPath();
		std::wstring cachePath = GetCachePath(srcPath);
		if (cachePath.empty())
			return false;

		tmpPath = cachePath + L".tmp";

		unsigned int pos = GetDataFolderPath().size();
		do
		{
			pos = folderPath.find_first_of(L"\\", pos + 1);
			std::wstring path;
			if (pos == std::string::npos)
				path = folderPath;
			else
				path = tmpPath.substr(0, pos).c_str();
			::CreateDirectory(path.c_str(), NULL);
		} while (pos != std::string::npos);

		if (!::PathFileExists(folderPath.c_str()))
			return false;

		return ::CopyFile(srcPath.c_str(), tmpPath.c_str(), FALSE);
	}

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
	Config *m_parent;
};

class RegistryConfig : public Config
{
public:
	RegistryConfig(bool systemWide = false)
		: m_systemWide(systemWide)
		, m_enableIncludeCache(false)
	{
		LoadIntValue(m_debug, L"Debug", m_systemWide);

		LoadStringValue(m_defaultBrowser,
			L"DefaultBrowser", m_systemWide);
		LoadStringValue(m_secondBrowser,
			L"SecondBrowser", m_systemWide);
		LoadStringValue(m_firefoxCommand,
			L"FirefoxCommand", m_systemWide);
		LoadStringValue(m_includePath,
			L"Include", m_systemWide);
		LoadIntValue(m_enableIncludeCache, L"EnableIncludeCache", m_systemWide);
		LoadIntValue(m_closeEmptyTab, L"CloseEmptyTab", m_systemWide);
		LoadIntValue(m_onlyOnAnchorClick, L"OnlyOnAnchorClick", m_systemWide);
		LoadIntValue(m_useRegex, L"UseRegex", m_systemWide);
		LoadZonePatterns(m_zonePatterns);
		LoadHostNamePatterns(m_hostNamePatterns);
		LoadURLPatterns(m_urlPatterns);

		dump();

		if (!m_includePath.empty())
			INIFileConfig::Include(*this, m_includePath, m_enableIncludeCache, m_debug > 0);
	}
	virtual ~RegistryConfig()
	{
	}

	virtual void dump()
	{
		if (m_debug <= 0)
			return;
		Config::dump();

		DebugLog(L"  Include: %ls", m_includePath.c_str());
		DebugLog(L"  EnableIncludeCache: %d", m_enableIncludeCache);
	}

	virtual std::wstring getName()
	{
		if (m_systemWide)
			return std::wstring(L"HKLM");
		else
			return std::wstring(L"HKCU");
	}

	static void LoadStringValue(
		std::wstring& value,
		const std::wstring& name,
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
		int& value,
		const std::wstring& name,
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
		SwitchingPatterns& patterns,
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

	void LoadZonePatterns(SwitchingPatterns& patterns)
	{
		LoadSwitchingPatterns(patterns, _T("ZonePatterns"), m_systemWide);
	}

	void LoadHostNamePatterns(SwitchingPatterns& patterns)
	{
		LoadSwitchingPatterns(patterns, _T("HostNamePatterns"), m_systemWide);
	}

	void LoadURLPatterns(SwitchingPatterns& patterns)
	{
		LoadSwitchingPatterns(patterns, _T("URLPatterns"), m_systemWide);
	}

public:
	bool m_systemWide;
	std::wstring m_includePath;
	int m_enableIncludeCache;
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

class BrowserSelector
{
public:
	BrowserSelector(const Config &config)
		: m_config(config)
	{
	}

	virtual ~BrowserSelector()
	{
	}

	const Config &m_config;

	bool isValidBrowserName(const std::wstring &browserName) const
	{
		if (browserName.empty())
			return false;
		if (browserName == L"firefox" && !m_config.m_firefoxCommand.empty())
			return true;
		return isInSystemPath(browserName);
	}

	std::wstring ensureValidBrowserName(
		const std::wstring *name = nullptr) const
	{
		if (name && isValidBrowserName(*name)) {
			return *name;
		} else if (name && name->empty() && isValidBrowserName(m_config.m_secondBrowser)) {
			if (m_config.m_debug > 0)
				DebugLog(L"Use second browser: %ls", m_config.m_secondBrowser.c_str());
			return m_config.m_secondBrowser;
		} else if (isValidBrowserName(m_config.m_defaultBrowser)) {
			if (m_config.m_debug > 0)
				DebugLog(L"Use default browser: %ls", m_config.m_defaultBrowser.c_str());
			return m_config.m_defaultBrowser;
		} else {
			if (m_config.m_debug > 0)
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

	bool matchRegex(const std::wstring &url, const std::wstring &pattern) const
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
		}
		catch (std::regex_error &e) {
			if (m_config.m_debug > 0)
				DebugLog(
					L"Failed to compile the regex! pattern: %ls, message: %ls",
					pattern.c_str(), e.what());
			return false;
		}
	}

	bool matchZone(const std::wstring &url, const std::wstring &zoneName) const
	{
		static const wchar_t *zones[] = { L"local", L"intra", L"trusted", L"internet", L"restricted" };
		DWORD index = -1;
		HRESULT ret;
		CComPtr<IInternetSecurityManager> securityManager;

		/* It is safe to call CoInitialize() several times, as long as
		 * we uninitialize COM just as many times.
		 *
		 * https://docs.microsoft.com/en-us/windows/win32/api/objbase/nf-objbase-coinitialize
		 */
		ret = CoInitialize(NULL);
		if (!SUCCEEDED(ret)) {
			DebugLog(L"Failed to call CoInitialize()");
			return false;
		}

		ret = securityManager.CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER);
		if (!SUCCEEDED(ret)) {
			DebugLog(L"Failed to initialize COM Object");
			CoUninitialize();
			return false;
		}

		/* Map URL to Internet Zone */
		ret = securityManager->MapUrlToZone(url.c_str(), &index, 0);

		/* Let's clean up COM here. */
		securityManager.Release();
		CoUninitialize();

		if (!SUCCEEDED(ret)) {
			DebugLog(L"Failed to map '%ls' to zone", url.c_str());
			return false;
		}

		if (index < 0 || 4 < index) {
			DebugLog(L"Unknown zone %i for '%ls'", index, url.c_str());
			return false;
		}

		return zoneName == zones[index];
	}

	bool matchURL(const std::wstring &url, const std::wstring &pattern) const
	{
		if (m_config.m_useRegex > 0)
			return matchRegex(url, pattern);
		else
			return matchSimpleWildCard(url, pattern);
	}

	std::wstring GetBrowserNameToOpenURL(const std::wstring &url) const
	{
		if (url.empty())
			return ensureValidBrowserName();

		SwitchingPatterns::const_iterator it;

		for (it = m_config.m_urlPatterns.begin(); it != m_config.m_urlPatterns.end(); it++) {
			const std::wstring &urlPattern = it->first;
			bool matched = matchURL(url, urlPattern);
			if (!matched)
				continue;
			if (m_config.m_debug > 0)
				DebugLog(L"Matched URL pattern: %ls Browser: %ls",
					it->first.c_str(), it->second.c_str());
			return ensureValidBrowserName(&it->second);
		}

		CUrl cURL;
		cURL.CrackUrl(url.c_str());
		LPCTSTR hostName = cURL.GetHostName();

		if (hostName && *hostName) {
			for (it = m_config.m_hostNamePatterns.begin(); it != m_config.m_hostNamePatterns.end(); it++) {
				const std::wstring &hostNamePattern = it->first;
				bool matched = matchURL(hostName, hostNamePattern);
				if (!matched)
					continue;
				if (m_config.m_debug > 0)
					DebugLog(L"Matched hostname pattern: %ls Browser: %ls",
						it->first.c_str(), it->second.c_str());
				return ensureValidBrowserName(&it->second);
			}
		}

		for (it = m_config.m_zonePatterns.begin(); it != m_config.m_zonePatterns.end(); it++) {
			const std::wstring &zone = it->first;
			bool matched = matchZone(url, zone);
			if (!matched)
				continue;
			if (m_config.m_debug > 0)
				DebugLog(L"Matched Zone pattern: %ls Browser: %ls",
					it->first.c_str(), it->second.c_str());
			return ensureValidBrowserName(&it->second);
		}

		if (m_config.m_debug > 0)
			DebugLog(L"Unmatched: %ls", url.c_str());

		return ensureValidBrowserName();
	}

	/*
	 * Create a new string buffer with std::wstring.
	 */
	static wchar_t *CreateStringBufferW(std::wstring &str)
	{
		size_t len = str.length() + 1;
		wchar_t *buf;

		buf = (wchar_t *)calloc(1, sizeof(wchar_t) * len);
		if (buf == NULL)
			return NULL;

		if (wcscpy_s(buf, len, str.c_str())) {
			free(buf);
			return NULL;
		}
		return buf;
	}

	/*
	 * Get the executable path to launch Firefox.
	 *
	 * This supports environmental variables in FirefoxCommand.
	 * (e.g. "%ProgramFiles%\Mozilla Firefox\firefox.exe")
	 */
	std::wstring GetFirefoxCommand(void) const
	{
		std::wstring cmd = m_config.m_firefoxCommand;
		wchar_t path[MAX_PATH];

		if (cmd.empty())
			return std::wstring(L"firefox.exe");

		if (!ExpandEnvironmentStringsW(cmd.c_str(), path, MAX_PATH)) {
			DebugLog(L"Failed to expand envs (err=%i)", GetLastError());
			DebugLog(L"Falling back to '%s'...", cmd.c_str());
			return cmd;
		}
		if (m_config.m_debug > 0)
			DebugLog(L"Expanded FirefoxCommand to '%s'", path);

		return std::wstring(path);
	}

	/*
	 * Open the given URL with Google Chrome.
	 *
	 * "flags" is passed to CreateProcess() as dwCreationFlags.
	 * Just use 0 except when you specially need other flags.
	 */
	bool OpenByChrome(const std::wstring &url, int flags) const
	{
		std::wstring cmd;
		std::wstring args(L"");
		wchar_t *buf;
		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		LoadBrowserPath(cmd, _T("chrome"));

		args += L"\"";
		args += cmd;
		args += L"\" -- \"";
		args += url;
		args += L"\"";

		buf = CreateStringBufferW(args);
		if (buf == NULL)
			return false;

		if (!CreateProcess(cmd.c_str(), buf, NULL, NULL, FALSE, flags, NULL, NULL, &si, &pi)) {
			DebugLog(L"CreateProcess failed (err=%i, cmd=%ls)", GetLastError(), cmd.c_str());
			free(buf);
			return false;
		}

		if (m_config.m_debug > 0)
			DebugLog(L"Launch chrome.exe (pid=%i)", pi.dwProcessId);

		/*
		 * Chrome seems to crash if the parent process exits too
		 * early. Let's wait 1.5 sec for startup.
		 */
		WaitForSingleObject(pi.hProcess, 1500);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		free(buf);
		return true;
	}

	bool OpenByModernBrowser(
		const std::wstring &browserName,
		const std::wstring &url,
		bool bypassElevationDialog = false) const
	{
		std::wstring command;
		std::wstring args(std::wstring(L"\"") + url + std::wstring(L"\""));

		if (bypassElevationDialog) {
			// Avoid showing the elevation warning dialog of IE. BrowserSelector.exe is
			// registered as Policy == 3 (don't show the dialog) by the installer.
			command = L"BrowserSelector.exe";
			args += std::wstring(L" --browser=") + browserName;
		} else {
			if (browserName == L"firefox") {
				command = GetFirefoxCommand();
			} else if (browserName == L"chrome") {
				return OpenByChrome(url, 0);
			}
		}

		HINSTANCE hInstance = 0;
		if (!command.empty())
			hInstance = ::ShellExecuteW(
				NULL, // HWND
				L"open",
				command.c_str(),
				args.c_str(),
				NULL, // Directory
				SW_SHOW);

		if (reinterpret_cast<int>(hInstance) > 32) {
			return true;
		} else {
			DebugLog(L"Failed to launch: code=%d, browser=%ls, url=%ls", hInstance, browserName.c_str(), url.c_str());
			return false;
		}
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

		if (!hConv)
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

};
