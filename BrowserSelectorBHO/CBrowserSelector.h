#pragma once
#include "resource.h"

#include "BrowserSelectorBHO_i.h"
#include <string>
#include <vector>

using namespace ATL;

class ATL_NO_VTABLE CBrowserSelector :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBrowserSelector, &CLSID_BrowserSelector>,
	public IObjectWithSiteImpl<CBrowserSelector>,
	public IDispatchImpl<IBrowserSelector, &IID_IBrowserSelector, &LIBID_BrowserSelectorBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CBrowserSelector()
		: m_secondBrowserName(L"firefox")
		, m_shouldCloseEmptyTab(true)
		, m_isEmptyTab(true)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERSELECTOR)


BEGIN_COM_MAP(CBrowserSelector)
	COM_INTERFACE_ENTRY(IBrowserSelector)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);
	STDMETHOD(Invoke)(
		DISPID dispidMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pdispparams,
		VARIANT *pvarResult,
		EXCEPINFO *pexcepinfo,
		UINT *puArgErr);

private:
	void LoadBHOSettings(bool systemWide = false);
	HRESULT Connect(void);
	HRESULT Disconnect(void);
	void OnBeforeNavigate2(
		IDispatch *pDisp,
		VARIANT *url,
		VARIANT *flags,
		VARIANT *targetFrameName,
		VARIANT *postData,
		VARIANT *headers,
		VARIANT_BOOL *cancel);
	bool IsEmptyURLPatterns(void);
	bool IsTopLevelFrame(IDispatch* pDisp);
	bool ShouldOpenByIE(const std::wstring &url);
	bool OpenBySecondBrowser(const std::wstring &url);

private:
	CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> m_webBrowser2;
	DWORD m_cookie;
	std::wstring m_secondBrowserName;
	std::wstring m_secondBrowserPath;
	std::vector<std::wstring> m_hostNamePatterns;
	std::vector<std::wstring> m_urlPatterns;
	bool m_shouldCloseEmptyTab;
	bool m_isEmptyTab;
};

OBJECT_ENTRY_AUTO(__uuidof(BrowserSelector), CBrowserSelector)
