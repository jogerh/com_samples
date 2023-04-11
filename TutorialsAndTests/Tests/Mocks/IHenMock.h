#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include <gmock/gmock.h>
#include <Interfaces/IHen.h>
#include <winrt/base.h>

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

/** Mock that shows how to implement local free-threaded COM objects with winrt */
struct FreeThreadedCluckObserver : winrt::implements<
                                                     FreeThreadedCluckObserver, // Implementation
                                                     IAsyncCluckObserver,       // Interface
                                                     IAgileObject               // Support calls from any thread
                                                    >
{
    MOCK_METHOD(HRESULT, OnCluck, (), (override));
};