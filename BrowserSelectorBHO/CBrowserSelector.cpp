#include "stdafx.h"
#include "CBrowserSelector.h"
#include <exdispid.h>
#include <string>
#include <atlutil.h>

using namespace std;

void CBrowserSelector::LoadFirefoxPath(void)
{
	CRegKey reg;

	LONG result = reg.Open(
		HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe"),
		KEY_READ);
	if (result != ERROR_SUCCESS)
		return;

	TCHAR path[MAX_PATH];
	ULONG pathSize = MAX_PATH;
	result = reg.QueryStringValue(NULL, path, &pathSize);
	if (result == ERROR_SUCCESS)
		m_secondBrowserPath = path;

	reg.Close();
}

static void LoadMatchingPatterns(
	vector<wstring> &patterns,
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

static void LoadFQDNPatterns(vector<wstring> &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("IntranetFQDNPatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("IntranetFQDNPatterns"));
}

static void LoadURLPatterns(vector<wstring> &patterns)
{
	bool systemWide = true;
	LoadMatchingPatterns(patterns, _T("IntranetURLPatterns"), systemWide);
	LoadMatchingPatterns(patterns, _T("IntranetURLPatterns"));
}

static bool IsIntranetURL(
	const wstring &url,
	const vector<wstring> &fqdnPatterns,
	const vector<wstring> &urlPatterns)
{
	static CComAutoCriticalSection symMatchSection;

	if (url.empty())
		return false;

	vector<wstring>::const_iterator it;
	CUrl cURL;
	cURL.CrackUrl(url.c_str());
	LPCTSTR hostName = cURL.GetHostName();

	if (hostName && *hostName) {
		for (it = fqdnPatterns.begin(); it != fqdnPatterns.end(); it++) {
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

HRESULT CBrowserSelector::FinalConstruct()
{
	LoadFirefoxPath();

	m_urlPatterns.push_back(L"about:*");
	LoadFQDNPatterns(m_fqdnPatterns);
	LoadURLPatterns(m_urlPatterns);

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

bool CBrowserSelector::IsTopLevelFrame(IDispatch* pDisp)
{
	if(!m_webBrowser2)
		return false;

	CComPtr<IDispatch> spDispatch;
	HRESULT hr = m_webBrowser2->QueryInterface(IID_IDispatch, (void**)&spDispatch);
	if (FAILED(hr))
		return false;

	return (pDisp == spDispatch);
}

void CBrowserSelector::OnBeforeNavigate2(
		IDispatch *pDisp,
		VARIANT *url,
		VARIANT *flags,
		VARIANT *targetFrameName,
		VARIANT *postData,
		VARIANT *headers,
		VARIANT_BOOL *cancel)
{
	CComVariant varURL(*url);
	varURL.ChangeType(VT_BSTR);
	wstring URL(varURL.bstrVal);

	if (!IsTopLevelFrame(pDisp))
		return;

	if (ShouldOpenByIE(URL))
		return;

	*cancel = VARIANT_TRUE;
	OpenBySecondBrowser(URL);
}

bool CBrowserSelector::ShouldOpenByIE(const wstring &url)
{
	static CComAutoCriticalSection symMatchSection;

	if (m_secondBrowserPath.empty())
		return false;
	return IsIntranetURL(url, m_fqdnPatterns, m_urlPatterns);
}

void CBrowserSelector::OpenBySecondBrowser(const wstring &url)
{
	if (m_secondBrowserPath.empty() || url.empty())
		return;

	::ShellExecute(NULL, _T("open"), _T("firefox.exe"), url.c_str(), NULL, SW_SHOW);
}
