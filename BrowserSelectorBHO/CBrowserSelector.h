#pragma once
#include "resource.h"
#include <exdispid.h>

#include "BrowserSelectorBHO_i.h"

using namespace ATL;

#include "../BrowserSelector/BrowserSelectorCommon.h"

class ATL_NO_VTABLE CBrowserSelector :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBrowserSelector, &CLSID_BrowserSelector>,
	public IObjectWithSiteImpl<CBrowserSelector>,
	public IDispatchImpl<IBrowserSelector, &IID_IBrowserSelector, &LIBID_BrowserSelectorBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispEventImpl<1, CBrowserSelector, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>
{
public:
	CBrowserSelector()
		: m_isEmptyTab(true)
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
	SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_ONQUIT, OnQuit)
END_SINK_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);

private:
	HRESULT Connect(void);
	HRESULT Disconnect(void);
	bool IsEmptyURLPatterns(void);
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
	void STDMETHODCALLTYPE OnQuit(
		LPDISPATCH pDisp);

private:
	CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> m_webBrowser2;
	DWORD m_cookie;
	Config m_config;
	bool m_shouldCloseEmptyTab;
	bool m_isEmptyTab;
};

OBJECT_ENTRY_AUTO(__uuidof(BrowserSelector), CBrowserSelector)
