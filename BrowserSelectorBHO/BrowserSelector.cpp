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
		OnBeforeNavigate2(
			pdispparams->rgvarg[6].pdispVal,
			pdispparams->rgvarg[5].pvarVal,
			pdispparams->rgvarg[4].pvarVal,
			pdispparams->rgvarg[3].pvarVal,
			pdispparams->rgvarg[2].pvarVal,
			pdispparams->rgvarg[1].pvarVal,
			pdispparams->rgvarg[0].pboolVal);
		break;
	default:
		break;
	}
	return S_OK;
}

void CBrowserSelector::OnBeforeNavigate2(
		IDispatch *pDisop,
		VARIANT *url,
		VARIANT *flags,
		VARIANT *targetFlagName,
		VARIANT *postData,
		VARIANT *headers,
		VARIANT_BOOL *cancel)
{
	if (true /* check the URL */)
		return;

	*cancel = VARIANT_TRUE;
	// Open the URL by an external browser
}
