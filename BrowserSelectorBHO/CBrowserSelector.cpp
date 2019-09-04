#include "stdafx.h"
#include "CBrowserSelector.h"
#include <exdispid.h>

using namespace std;

HRESULT CBrowserSelector::FinalConstruct()
{
	m_config.LoadAll(_AtlBaseModule.GetResourceInstance());
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
	return DispEventAdvise(m_webBrowser2);
}

HRESULT CBrowserSelector::Disconnect(void)
{
	return DispEventUnadvise(m_webBrowser2);
}

STDMETHODIMP CBrowserSelector::SetSite(IUnknown *pUnkSite)
{
	m_webBrowser2 = pUnkSite;
	if (m_webBrowser2 == NULL)
		return E_INVALIDARG;
	return Connect();
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

void STDMETHODCALLTYPE CBrowserSelector::OnBeforeNavigate2(
		LPDISPATCH pDisp,
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
		if (m_config.m_closeEmptyTab && m_isEmptyTab)
			m_webBrowser2->Quit();
	} else {
		// Fall back to IE
		*cancel = VARIANT_FALSE;
	}
}

void STDMETHODCALLTYPE CBrowserSelector::OnNavigateComplete2(
		LPDISPATCH pDisp,
		VARIANT *URL)
{
	if (!IsTopLevelFrame(pDisp))
		return;
	CComPtr<IDispatch> pDispDocument;
	m_webBrowser2->get_Document(&pDispDocument);
	CComQIPtr<IHTMLDocument3, &IID_IHTMLDocument3> document(pDispDocument);
}

void STDMETHODCALLTYPE CBrowserSelector::OnQuit(LPDISPATCH pDisp)
{
	Disconnect();
}

wstring CBrowserSelector::GetBrowserNameToOpenURL(const wstring &url)
{
	static CComAutoCriticalSection symMatchSection;

	if (IsEmptyURLPatterns())
		return wstring(L"ie");
	return ::GetBrowserNameToOpenURL(url, m_config);
}
