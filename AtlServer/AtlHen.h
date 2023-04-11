#pragma once
#include "resource.h" // main symbols
#include <AtlServer/AtlServer.h>

using namespace ATL;

class ATL_NO_VTABLE AtlHen :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AtlHen, &CLSID_AtlHen>,
    public IHen
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_HEN)

    BEGIN_COM_MAP(AtlHen)
        COM_INTERFACE_ENTRY(IHen)
    END_COM_MAP()

    HRESULT Cluck() override;
    HRESULT CluckAsync(IAsyncCluckObserver* cluckObserver) override;

private:
    unsigned long m_myThreadId = GetCurrentThreadId();
};

OBJECT_ENTRY_AUTO(CLSID_AtlHen, AtlHen)



class ATL_NO_VTABLE FreeThreadedHen :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<FreeThreadedHen, &CLSID_FreeThreadedHen>,
    public IHen
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_HEN)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(FreeThreadedHen)
        COM_INTERFACE_ENTRY(IHen)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_marshaler)
    END_COM_MAP()

    HRESULT FinalConstruct();

    HRESULT Cluck() override;
    HRESULT CluckAsync(IAsyncCluckObserver* cluckObserver) override;

private:
    CComPtr<IUnknown> m_marshaler;
    unsigned long m_myThreadId = GetCurrentThreadId();
};

OBJECT_ENTRY_AUTO(CLSID_FreeThreadedHen, FreeThreadedHen)



class ATL_NO_VTABLE AtlCluckObserver : public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AtlCluckObserver, &CLSID_AtlCluckObserver>,
    public IAsyncCluckObserver
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_ASYNCCLUCKOBSERVER)

    BEGIN_COM_MAP(AtlCluckObserver)
        COM_INTERFACE_ENTRY(IAsyncCluckObserver)
    END_COM_MAP()

    HRESULT OnCluck() override;

private:
    unsigned long m_myThreadId = GetCurrentThreadId();
};

OBJECT_ENTRY_AUTO(CLSID_AtlCluckObserver, AtlCluckObserver)