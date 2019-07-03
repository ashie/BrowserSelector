#pragma once
#include "resource.h"

#include "BrowserSelectorBHO_i.h"

using namespace ATL;

class ATL_NO_VTABLE CBrowserSelector :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBrowserSelector, &CLSID_BrowserSelector>,
	public IObjectWithSiteImpl<CBrowserSelector>,
	public IDispatchImpl<IBrowserSelector, &IID_IBrowserSelector, &LIBID_BrowserSelectorBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CBrowserSelector()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERSELECTOR)


BEGIN_COM_MAP(CBrowserSelector)
	COM_INTERFACE_ENTRY(IBrowserSelector)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);
	STDMETHOD(Invoke)(
		DISPID dispidMember, 
		REFIID riid,
		LCID lcid, 
		WORD wFlags, 
		DISPPARAMS* pdispparams, 
		VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, 
		UINT* puArgErr);

private:
	CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> m_webBrowser2;

};

OBJECT_ENTRY_AUTO(__uuidof(BrowserSelector), CBrowserSelector)
