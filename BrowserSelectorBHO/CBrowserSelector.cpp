#include "stdafx.h"
#include "CBrowserSelector.h"
#include <exdispid.h>

using namespace std;

HRESULT CBrowserSelector::FinalConstruct()
{
	m_config.LoadAll(_AtlBaseModule.GetResourceInstance());
	SwitchingPatterns &patterns(m_config.m_urlPatterns);
	if (m_config.m_useRegex) {
		patterns.insert(patterns.begin(), SwitchingPattern(L"^about:.*", L"ie"));
		patterns.insert(patterns.begin(), SwitchingPattern(L"^res://.*", L"ie"));
	} else {
		patterns.insert(patterns.begin(), SwitchingPattern(L"about:*", L"ie"));
		patterns.insert(patterns.begin(), SwitchingPattern(L"res://*", L"ie"));
	}

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

	if (m_isEmptyFrame)
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

void CBrowserSelector::DoNavigate(BSTR url, VARIANT_BOOL *cancel)
{
	wstring URL(url);

	*cancel = VARIANT_FALSE;

	if (m_config.m_onlyOnAnchorClick) {
		std::wstring lastClickedURL = m_lastClickedURL;
		m_lastClickedURL.clear();
		int timeDiff = ::GetTickCount() - m_lastClickedTime;
		if (abs(timeDiff) > 100)
			return;
		if (lastClickedURL.empty())
			return;
	}

	wstring browserName = GetBrowserNameToOpenURL(URL);
	if (browserName == L"ie")
		return;

	bool succeeded = OpenByModernBrowser(browserName, URL);
	*cancel= succeeded ? VARIANT_TRUE : VARIANT_FALSE;
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

	DoNavigate(url->bstrVal, cancel);

	if (*cancel) {
		if (m_config.m_closeEmptyTab && m_isEmptyFrame)
			m_webBrowser2->Quit();
		else
			ConnectDocumentEvents();
	}
}

void STDMETHODCALLTYPE CBrowserSelector::OnNavigateComplete2(
		LPDISPATCH pDisp,
		VARIANT *url)
{
	wstring URL(url->bstrVal ? url->bstrVal : L"");
	if (URL.size() > 0 && URL != L"about:blank" && URL != L"about:NewsFeed")
		m_isEmptyFrame = false;
}

void STDMETHODCALLTYPE CBrowserSelector::OnDocumentComplete(
		LPDISPATCH pDisp,
		VARIANT *url)
{
	if (!IsTopLevelFrame(pDisp))
		return;

	ConnectDocumentEvents();
}

void STDMETHODCALLTYPE CBrowserSelector::OnNewWindow3(
		LPDISPATCH *pDisp,
		VARIANT_BOOL *cancel,
		DWORD flags,
		BSTR urlContext,
		BSTR url)
{
	DoNavigate(url, cancel);

	if (*cancel) {
		if (m_config.m_closeEmptyTab && m_isEmptyFrame)
			m_webBrowser2->Quit();
	}
}

void STDMETHODCALLTYPE CBrowserSelector::OnQuit(LPDISPATCH pDisp)
{
	DisconnectDocumentEvents();
	DisconnectBrowserEvents();
}

static void GetLink(std::wstring &url, IHTMLEventObj *pEventObj)
{
	if (!pEventObj)
		return;

	long button = 0;
	HRESULT hr = pEventObj->get_button(&button);
	if (FAILED(hr) || button != 1)
		return;

	CComPtr<IHTMLElement> element;
	pEventObj->get_srcElement(&element);

	while (element) {
		CComBSTR tagName;
		element->get_tagName(&tagName);

		if(tagName == "a" || tagName == "A") {
			CComVariant v;
			hr = element->getAttribute(L"href", 0, &v);
			if (SUCCEEDED(hr) && v.bstrVal)
				url = v.bstrVal;
			break;
		}

		hr = element->get_parentElement(&element);
		if (FAILED(hr))
			break;
	}
}

bool STDMETHODCALLTYPE CBrowserSelector::OnMouseDown(IHTMLEventObj *pEventObj)
{
	GetLink(m_lastPressedURL, pEventObj);
	return true;
}

bool STDMETHODCALLTYPE CBrowserSelector::OnMouseUp(IHTMLEventObj *pEventObj)
{
	std::wstring lastPressedURL(m_lastPressedURL), url;
	if (m_lastPressedURL.empty())
		return true;

	m_lastPressedURL.clear();

	GetLink(m_lastClickedURL, pEventObj);

	if (!m_lastClickedURL.empty() && lastPressedURL != m_lastClickedURL)
		m_lastClickedURL.clear();
	m_lastClickedTime = ::GetTickCount();
	return true;
}

wstring CBrowserSelector::GetBrowserNameToOpenURL(const wstring &url)
{
	return ::GetBrowserNameToOpenURL(url, m_config);
}
