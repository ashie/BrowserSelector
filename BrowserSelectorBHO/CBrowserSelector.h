#pragma once
#include "resource.h"
#include <exdispid.h>
#include <mshtmdid.h>
#include <mshtml.h>

#include "BrowserSelectorBHO_i.h"

using namespace ATL;

#include "../BrowserSelector/BrowserSelectorCommon.h"

class ATL_NO_VTABLE CBrowserSelector :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBrowserSelector, &CLSID_BrowserSelector>,
	public IObjectWithSiteImpl<CBrowserSelector>,
	public IDispatchImpl<IBrowserSelector, &IID_IBrowserSelector, &LIBID_BrowserSelectorBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispEventImpl<1, CBrowserSelector, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>,
	public IDispEventImpl<2, CBrowserSelector, &DIID_HTMLDocumentEvents2, &LIBID_MSHTML, 4, 0>
{
public:
	CBrowserSelector()
		: m_isEmptyTab(true)
		, m_lastClickedTime(0)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERSELECTOR)


BEGIN_COM_MAP(CBrowserSelector)
	COM_INTERFACE_ENTRY(IBrowserSelector)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

BEGIN_SINK_MAP(CBrowserSelector)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_ONQUIT, OnQuit)
	SINK_ENTRY_EX(2, DIID_HTMLDocumentEvents2, DISPID_HTMLDOCUMENTEVENTS2_ONMOUSEUP, OnMouseUp)
END_SINK_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);

private:
	HRESULT ConnectBrowserEvents(void);
	HRESULT DisconnectBrowserEvents(void);
	HRESULT ConnectDocumentEvents(void);
	HRESULT DisconnectDocumentEvents(void);
	bool IsTopLevelFrame(IDispatch* pDisp);
	std::wstring GetBrowserNameToOpenURL(const std::wstring &url);

	void STDMETHODCALLTYPE OnBeforeNavigate2(
		LPDISPATCH pDisp,
		VARIANT* URL,
		VARIANT* Flags,
		VARIANT* TargetFrameName,
		VARIANT* PostData,
		VARIANT* Headers,
		VARIANT_BOOL* Cancel);
	void STDMETHODCALLTYPE OnNavigateComplete2(
		LPDISPATCH pDisp,
		VARIANT* URL);
	void STDMETHODCALLTYPE OnDocumentComplete(
		LPDISPATCH pDisp,
		VARIANT* URL);
	void STDMETHODCALLTYPE OnQuit(
		LPDISPATCH pDisp);
	bool STDMETHODCALLTYPE OnMouseUp(IHTMLEventObj *pEventObj);

private:
	CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> m_webBrowser2;
	Config m_config;
	bool m_shouldCloseEmptyTab;
	bool m_isEmptyTab;
	DWORD m_lastClickedTime;
	std::wstring m_lastClickedURL;
};

OBJECT_ENTRY_AUTO(__uuidof(BrowserSelector), CBrowserSelector)
