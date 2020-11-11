class CBrowserSelectorBHOModule : public ATL::CAtlDllModuleT< CBrowserSelectorBHOModule >
{
public :
	DECLARE_LIBID(LIBID_BrowserSelectorBHOLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BROWSERSELECTORBHO, "{4FFE7D2B-2618-4867-AC9F-C7E35ED26F8C}")

	virtual HRESULT AddCommonRGSReplacements(IRegistrarBase *pRegistrar);
};

extern class CBrowserSelectorBHOModule _AtlModule;
