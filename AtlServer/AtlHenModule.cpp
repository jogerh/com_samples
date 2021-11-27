// AtlHen.cpp : Implementation of DLL Exports.


#include "pch.h"

#include "framework.h"
#include "resource.h"
#include <AtlHenLib/AtlHenLib.h>
#include <AtlHenLib/AtlHenLib_i.c>

class AtlHenModule : public ATL::CAtlDllModuleT<AtlHenModule>
{
public:
    DECLARE_LIBID(LIBID_AtlHenLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ATLHENLIB, "{6ed1b1aa-807b-4a28-87b6-fcdc18ab8dc3}")
};

AtlHenModule s_atlModule;

using namespace ATL;

STDAPI DllCanUnloadNow(void)
{
    return s_atlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
    return s_atlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
    return s_atlModule.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
    return s_atlModule.DllUnregisterServer();
}

