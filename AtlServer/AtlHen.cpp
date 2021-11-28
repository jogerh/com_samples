#include "pch.h"
#include "AtlHen.h"
#include <comdef.h>

HRESULT AtlHen::Cluck()
{
    return S_OK;
}

HRESULT AtlHen::CluckAsync(IAsyncCluckObserver* cluckObserver)
{
    return cluckObserver->OnCluck();
}

HRESULT AtlCluckObserver::OnCluck()
{
    printf("Cluck\n");
    return S_OK;
}
