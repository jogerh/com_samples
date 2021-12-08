#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>

struct ComException
{
    HRESULT result;

    explicit ComException(HRESULT const value) : result(value) {}
};

inline void HR(HRESULT const result)
{
    if (S_OK != result) throw ComException(result);
}

/** Helper function to create instances of ATL COM objects */
template <typename T>
CComPtr<T> make_self() {
    CComObject<T>* tmp = nullptr;
    HR(CComObject<T>::CreateInstance(&tmp));
    return CComPtr<T>(static_cast<T*>(tmp));
}

enum class Apartment
{
    MultiThreaded = COINIT_MULTITHREADED,
    SingleThreaded = COINIT_APARTMENTTHREADED,
};

struct ComRuntime
{
    explicit ComRuntime(Apartment apartment = Apartment::SingleThreaded)
    {
        HR(CoInitializeEx(nullptr, static_cast<DWORD>(apartment)));
    }
    ~ComRuntime()
    {
        CoUninitialize();
    }
};


template <typename T>
class AgilePtr final
{
public:
    explicit AgilePtr(T* ifPointer)
    {
        HR(RoGetAgileReference(AGILEREFERENCE_DEFAULT, __uuidof(ifPointer), ifPointer, &m_agileRef));
    }

    ~AgilePtr()
    {
        m_agileRef.Release();
    }

    // Prevent moving the agile reference into another thread. This can cause memory leaks
    AgilePtr(const AgilePtr&) = default;
    AgilePtr(AgilePtr&& in) = delete;
    AgilePtr& operator=(const AgilePtr& rhs) = default;
    AgilePtr& operator=(AgilePtr&& rhs) = delete;

    CComPtr<T> Get() const
    {
        CComPtr<T> ifPointer;
        HR(m_agileRef->Resolve(__uuidof(T), reinterpret_cast<void**>(&ifPointer)));
        return ifPointer;
    }

private:
    CComPtr<IAgileReference> m_agileRef;
};

// Auto-link
#ifndef COM_UTILITY_BUILD
#pragma comment(lib, "ComUtility.lib")
#endif
