#include "pch.h"
#include "AtlHen.h"
#include <comdef.h>
#include <cassert>
#include <future>
#include <ComUtility/Utility.h>
#include <winrt/base.h>

HRESULT AtlHen::Cluck()
{
    assert(m_myThreadId == GetCurrentThreadId());
    return S_OK;
}

HRESULT AtlHen::CluckAsync(IAsyncCluckObserver* cluckObserver)
{
    assert(m_myThreadId == GetCurrentThreadId());
    return cluckObserver->OnCluck();
}

HRESULT FreeThreadedHen::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_marshaler);
}

HRESULT FreeThreadedHen::Cluck()
{
    assert(m_myThreadId == GetCurrentThreadId());
    return S_OK;
}

HRESULT FreeThreadedHen::CluckAsync(IAsyncCluckObserver* cluckObserver)
{
    winrt::com_ptr<IAsyncCluckObserver> observer;
    observer.copy_from(cluckObserver);

    auto result = std::async(std::launch::async, [observer] {
        ComRuntime comRuntime{ Apartment::MultiThreaded };
        return observer->OnCluck();
    });

    return result.get();
}

HRESULT AtlCluckObserver::OnCluck()
{
    assert(m_myThreadId == GetCurrentThreadId());
    printf("Cluck\n");
    return S_OK;
}
