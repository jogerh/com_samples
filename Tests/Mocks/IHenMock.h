#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include <gmock/gmock.h>
#include <Interfaces/IHen.h>

/** Mock that shows how to create local COM objects using ATL */
struct IAsyncCluckObserverMock :
    CComObjectRootEx<CComMultiThreadModel>,
    CComCoClass<IAsyncCluckObserverMock>,
    IAsyncCluckObserver
{
    BEGIN_COM_MAP(IAsyncCluckObserverMock)
        COM_INTERFACE_ENTRY(IAsyncCluckObserver)
    END_COM_MAP()

    MOCK_METHOD(HRESULT, OnCluck, (), (override));
};
