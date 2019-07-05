#include "stdafx.h"
#include "BrowserSelector.h"
#include <exdispid.h>
#include <string>
#include <DbgHelp.h>

using namespace std;

HRESULT CBrowserSelector::FinalConstruct()
{
	m_secondBrowserPath = L"C:\\Program Files\\Mozilla Firefox\\firefox.exe";

	CRegKey reg;
	LONG result = reg.Open(
		HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\ClearCode\\BrowserSelector\\IntranetURLPatterns"),
		KEY_READ);

	for (DWORD idx = 0; result == ERROR_SUCCESS; idx++) {
		TCHAR value[256];
		DWORD valueLen = 256;
		result = ::RegEnumValue(reg.m_hKey, idx,value, &valueLen, NULL, NULL, NULL, NULL);
		if (result != ERROR_SUCCESS)
			continue;
		m_urlPatterns.push_back(value);
	}

	reg.Close();

	return S_OK;
}

void CBrowserSelector::FinalRelease()
{
}

static HRESULT GetConnectionPoint(
	CComPtr<IWebBrowser2> browser,
	CComPtr<IConnectionPoint> &connectionPoint)
{
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> container(browser);
	if (container == NULL)
		return E_POINTER;

	HRESULT hr = container->FindConnectionPoint(DIID_DWebBrowserEvents2, &connectionPoint);
	if (!SUCCEEDED(hr))
		return hr;

	return S_OK;
}

HRESULT CBrowserSelector::Connect(void)
{
	CComPtr<IConnectionPoint> connectionPoint;
	HRESULT hr = GetConnectionPoint(m_webBrowser2, connectionPoint);
	if (!SUCCEEDED(hr))
		return hr;
	return connectionPoint->Advise((IDispatch*)this, &m_cookie);
}

HRESULT CBrowserSelector::Disconnect(void)
{
	CComPtr<IConnectionPoint> connectionPoint;
	HRESULT hr = GetConnectionPoint(m_webBrowser2, connectionPoint);
	if (!SUCCEEDED(hr))
		return hr;
	return connectionPoint->Unadvise(m_cookie);
}

STDMETHODIMP CBrowserSelector::SetSite(IUnknown *pUnkSite)
{
	m_webBrowser2 = pUnkSite;
	if (m_webBrowser2 == NULL)
		return E_INVALIDARG;
	return Connect();
}

STDMETHODIMP CBrowserSelector::Invoke(
	DISPID dispidMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS *pdispparams,
	VARIANT *pvarResult,
	EXCEPINFO *pexcepinfo,
	UINT *puArgErr)
{
	switch(dispidMember) {
	case DISPID_BEFORENAVIGATE2:
		OnBeforeNavigate2(
			pdispparams->rgvarg[6].pdispVal,
			pdispparams->rgvarg[5].pvarVal,
			pdispparams->rgvarg[4].pvarVal,
			pdispparams->rgvarg[3].pvarVal,
			pdispparams->rgvarg[2].pvarVal,
			pdispparams->rgvarg[1].pvarVal,
			pdispparams->rgvarg[0].pboolVal);
		break;
	case DISPID_QUIT:
		return Disconnect();
	default:
		break;
	}
	return S_OK;
}

void CBrowserSelector::OnBeforeNavigate2(
		IDispatch *pDisop,
		VARIANT *url,
		VARIANT *flags,
		VARIANT *targetFlagName,
		VARIANT *postData,
		VARIANT *headers,
		VARIANT_BOOL *cancel)
{
	CComVariant varURL(*url);
	varURL.ChangeType(VT_BSTR);
	wstring URL(varURL.bstrVal);

	if (ShouldOpenBySecondBrowser(URL)) {
		*cancel = VARIANT_TRUE;
		OpenBySecondBrowser(URL);
	}
}

bool CBrowserSelector::ShouldOpenBySecondBrowser(const wstring &url)
{
	static CComAutoCriticalSection symMatchSection;
	vector<wstring>::iterator it = m_urlPatterns.begin();

	for (; it != m_urlPatterns.end(); it++) {
		symMatchSection.Lock();
		BOOL matched = SymMatchStringW(url.c_str(), it->c_str(), FALSE);
		symMatchSection.Unlock();
		if (matched)
			return true;
	}

	return false;
}

static void BuildCommandLine(const wstring &browserPath, const wstring &url, wstring &commandLine)
{
	commandLine = L"\"";
	commandLine += browserPath;
	commandLine += L"\" \"";
	commandLine += url;
	commandLine += L"\"";
}

void CBrowserSelector::OpenBySecondBrowser(const wstring &url)
{
	if (m_secondBrowserPath.empty() || url.empty())
		return;

	wstring commandLine;
	BuildCommandLine(m_secondBrowserPath, url, commandLine);

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInfo = { 0 };
	::CreateProcessW(
		m_secondBrowserPath.c_str(),
		(LPWSTR)commandLine.c_str(),
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_PROCESS_GROUP,
		NULL,
		NULL,
		&startupInfo,
		&processInfo);
}
