#include "pch.h"
#include "AtlHen.h"
#include <comdef.h>
#include <cassert>

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
    return cluckObserver->OnCluck();
}

HRESULT AtlCluckObserver::OnCluck()
{
    assert(m_myThreadId == GetCurrentThreadId());
    printf("Cluck\n");
    return S_OK;
}
