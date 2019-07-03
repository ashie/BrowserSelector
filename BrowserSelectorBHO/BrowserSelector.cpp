#include "stdafx.h"
#include "BrowserSelector.h"
#include <exdispid.h>

STDMETHODIMP CBrowserSelector::SetSite(IUnknown *pUnkSite) {
	m_webBrowser2 = pUnkSite;
	if (m_webBrowser2 == NULL)
		return E_INVALIDARG;
	return S_OK;
}

STDMETHODIMP CBrowserSelector::Invoke(
		DISPID dispidMember, 
		REFIID riid,
		LCID lcid, 
		WORD wFlags, 
		DISPPARAMS* pdispparams, 
		VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, 
		UINT* puArgErr)
{
	switch(dispidMember) {
	case DISPID_BEFORENAVIGATE2:
		break;
	default:
		break;
	}
	return S_OK;
}
