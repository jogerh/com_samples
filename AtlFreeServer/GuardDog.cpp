#include "pch.h"
#include "Include/AtlFreeServer/GuardDog.h"
#include <ComUtility/Utility.h>
#include <future>
#include <Interfaces/IDog.h>
#include <Interfaces/IPostman.h>
#include <cassert>

using namespace Microsoft::WRL;

static long s_serverLock;

struct GuardDog : IDog
{
    long m_count;

    GuardDog() : m_count(0)
    {
        _InterlockedIncrement(&s_serverLock);
    }

    ~GuardDog()
    {
        _InterlockedDecrement(&s_serverLock);
    }

    ULONG __stdcall AddRef() override
    {
        return _InterlockedIncrement(&m_count);
    }

    ULONG __stdcall Release() override
    {
        ULONG result = _InterlockedDecrement(&m_count);

        if (0 == result)
        {
            delete this;
        }

        return result;
    }

    HRESULT __stdcall QueryInterface(IID const & id,
                                     void ** result) override
    {
        assert(result);

        if (id == __uuidof(IDog) ||
            id == __uuidof(IUnknown))
        {
            *result = static_cast<IDog *>(this);
        }
        else
        {
            *result = 0;
            return E_NOINTERFACE;
        }

        static_cast<IUnknown *>(*result)->AddRef();
        return S_OK;
    }

    HRESULT Sit() override
    {
        printf("%s", "Sitting!\n");
        return S_OK;
    }

    HRESULT Bite(IPostman* postman) override
    {
        return postman->OnBitten();
    }
};

struct PuppyFarm : IClassFactory
{
    ULONG __stdcall AddRef() override
    {
        return 2;
    }

    ULONG __stdcall Release() override
    {
        return 1;
    }

    HRESULT __stdcall QueryInterface(IID const & id,
                                     void ** result) override
    {
        assert(result);

        if (id == __uuidof(IClassFactory) ||
            id == __uuidof(IUnknown))
        {
            *result = static_cast<IClassFactory *>(this);
        }
        else
        {
            *result = 0;
            return E_NOINTERFACE;
        }

        return S_OK;
    }

    HRESULT __stdcall CreateInstance(IUnknown * outer,
                                     IID const & iid,
                                     void ** result) override
    {
        assert(result);
        *result = nullptr;

        if (outer)
        {
            return CLASS_E_NOAGGREGATION;
        }

        auto dog = new (std::nothrow) GuardDog;

        if (!dog)
        {
            return E_OUTOFMEMORY;
        }

        dog->AddRef();
        auto hr = dog->QueryInterface(iid, result);
        dog->Release();

        return hr;
    }

    HRESULT __stdcall LockServer(BOOL lock) override
    {
        if (lock)
        {
            _InterlockedIncrement(&s_serverLock);
        }
        else
        {
            _InterlockedDecrement(&s_serverLock);
        }

        return S_OK;
    }
};

// The following function is implemented in the auto-generated dlldata.c file from the Interfaces project
extern "C"
HRESULT __stdcall ProxyDllGetClassObject(CLSID const & clsid,
                                         IID const & iid,
                                         void ** result);

HRESULT __stdcall DllGetClassObject(CLSID const & clsid,
                                    IID const & iid,
                                    void ** result)
{
    // If you want to play with tests or tutorials involving out-of-process activation,
    // uncomment the code below. This allows attaching to the dllhost.exe process once
    // CoCreateInstance(... CLSCTX_LOCAL_SERVER ...) has been called.
    //
    // To find the process id of the dllhost.exe that hosts the COM server, enable the 'Command line'
    // column in task manager, and look for the process with command line containing
    //
    //    /Processid:{2b083fea-3681-4c9b-9ed1-3e866124a58d}
    //
    // The guid is the 'AppID' that was used when registering the AtlFreeServer

    //while (!IsDebuggerPresent())
    //    Sleep(100);

    assert(result);
    *result = nullptr;

    auto hr = ProxyDllGetClassObject(clsid,
                                     iid,
                                     result);

    if (CLASS_E_CLASSNOTAVAILABLE != hr)
    {
        return hr;
    }

    if (__uuidof(GuardDog) == clsid)
    {
        static PuppyFarm farm;

        return farm.QueryInterface(iid, result);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

// The following function is implemented in the auto-generated dlldata.c file from the Interfaces project
extern "C"
HRESULT __stdcall ProxyDllCanUnloadNow();

HRESULT __stdcall DllCanUnloadNow()
{
    auto hr = ProxyDllCanUnloadNow();

    if (S_OK != hr)
    {
        return hr;
    }

    return s_serverLock ? S_FALSE : S_OK;
}
