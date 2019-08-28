#include "stdafx.h"
#include "CBrowserSelector.h"
#include <exdispid.h>

using namespace std;

void CBrowserSelector::LoadBHOSettings(bool systemWide)
{
	CRegKey reg;
	HKEY keyParent = systemWide ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	CString regKeyName(_T("SOFTWARE\\ClearCode\\BrowserSelector"));
	LONG result = reg.Open(keyParent, regKeyName, KEY_READ);
	if (result == ERROR_SUCCESS) {
		DWORD value;
		result = reg.QueryDWORDValue(L"CloseEmptyTab", value);
		if (result == ERROR_SUCCESS)
			m_shouldCloseEmptyTab = value ? true : false;
	}
	reg.Close();
}

HRESULT CBrowserSelector::FinalConstruct()
{
	m_config.LoadAll();
	MatchingPatterns &patterns(m_config.m_urlPatterns);
	patterns.insert(patterns.begin(), MatchingPattern(L"about:*", L"ie"));

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

bool CBrowserSelector::IsEmptyURLPatterns(void)
{
	return
		m_config.m_hostNamePatterns.empty() &&
		m_config.m_urlPatterns.size() == 1;
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

	wstring browserName = GetBrowserNameToOpenURL(URL);

	if (browserName == L"ie") {
		if (URL.size() > 0 && URL != L"about:blank" /* && URL != L"about:NewsFeed" */)
			m_isEmptyTab = false;
		return;
	}

	*cancel = VARIANT_TRUE;
	bool succeeded = OpenByModernBrowser(browserName, URL);

	if (succeeded) {
		if (m_shouldCloseEmptyTab && m_isEmptyTab)
			m_webBrowser2->Quit();
	} else {
		// Fall back to IE
		*cancel = VARIANT_FALSE;
	}
}

wstring CBrowserSelector::GetBrowserNameToOpenURL(const wstring &url)
{
	static CComAutoCriticalSection symMatchSection;

	if (IsEmptyURLPatterns())
		return wstring(L"ie");
	return ::GetBrowserNameToOpenURL(url, m_config);
}
