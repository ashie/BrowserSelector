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

HRESULT CBrowserSelector::ConnectBrowserEvents(void)
{
	return IDispEventImpl<1, CBrowserSelector, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>::DispEventAdvise(m_webBrowser2);
}

HRESULT CBrowserSelector::DisconnectBrowserEvents(void)
{
	return IDispEventImpl<1, CBrowserSelector, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>::DispEventUnadvise(m_webBrowser2);
}

HRESULT CBrowserSelector::ConnectDocumentEvents(void)
{
	if (!m_config.m_onlyOnAnchorClick)
		return S_OK;

	CComPtr<IDispatch> pDispDocument;
	m_webBrowser2->get_Document(&pDispDocument);
	CComQIPtr<IHTMLDocument3, &IID_IHTMLDocument3> document(pDispDocument);
	if (!document)
		return S_OK;
	return IDispEventImpl<2, CBrowserSelector, &DIID_HTMLDocumentEvents2, &LIBID_MSHTML, 4, 0>::DispEventAdvise(document);
}

HRESULT CBrowserSelector::DisconnectDocumentEvents(void)
{
	if (!m_config.m_onlyOnAnchorClick)
		return S_OK;

	CComPtr<IDispatch> pDispDocument;
	m_webBrowser2->get_Document(&pDispDocument);
	CComQIPtr<IHTMLDocument3, &IID_IHTMLDocument3> document(pDispDocument);
	if (!document)
		return S_OK;
	return IDispEventImpl<2, CBrowserSelector, &DIID_HTMLDocumentEvents2, &LIBID_MSHTML, 4, 0>::DispEventUnadvise(document);
}

STDMETHODIMP CBrowserSelector::SetSite(IUnknown *pUnkSite)
{
	m_webBrowser2 = pUnkSite;
	if (m_webBrowser2 == NULL)
		return E_INVALIDARG;
	return ConnectBrowserEvents();
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
	if (!IsTopLevelFrame(pDisp))
		return;

	DisconnectDocumentEvents();

	bool isClicked = m_isClicked;
	m_isClicked = false;

	wstring URL(url->bstrVal);
	wstring browserName = GetBrowserNameToOpenURL(URL);

	if (browserName == L"ie") {
		if (URL.size() > 0 && URL != L"about:blank" /* && URL != L"about:NewsFeed" */)
			m_isEmptyTab = false;
		return;
	}

	if (m_config.m_onlyOnAnchorClick && !isClicked)
		return;

	*cancel = VARIANT_TRUE;
	bool succeeded = OpenByModernBrowser(browserName, URL);

	if (succeeded) {
		if (m_config.m_closeEmptyTab && m_isEmptyTab)
			m_webBrowser2->Quit();
		else
			ConnectDocumentEvents();
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
	ConnectDocumentEvents();
}

void STDMETHODCALLTYPE CBrowserSelector::OnQuit(LPDISPATCH pDisp)
{
	DisconnectDocumentEvents();
	DisconnectBrowserEvents();
}

void STDMETHODCALLTYPE CBrowserSelector::OnClick(IHTMLEventObj *pEventObj)
{
	HRESULT hr;

	IHTMLElement *element = nullptr;
	pEventObj->get_srcElement(&element);

	wstring debugString(L"CBrowserSelector::OnClick: ");

	while (element) {
		CComBSTR tagName;
		element->get_tagName(&tagName);

		if(tagName == "a" || tagName == "A") {
			CComVariant v;
			hr = element->getAttribute(L"href", 0, &v);
			if (SUCCEEDED(hr)) {
				// TODO: Should be compared at BeforeNavigate2
				debugString += v.bstrVal;
				m_isClicked = true;
			}
			element->Release();
			break;
		}

		IHTMLElement *parentElement = nullptr;
		element->get_parentElement(&parentElement);
		element->Release();
		element = parentElement;
	}

	OutputDebugString(debugString.c_str());
}

wstring CBrowserSelector::GetBrowserNameToOpenURL(const wstring &url)
{
	static CComAutoCriticalSection symMatchSection;

	if (IsEmptyURLPatterns())
		return wstring(L"ie");
	return ::GetBrowserNameToOpenURL(url, m_config);
}
