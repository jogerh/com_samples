#pragma once
#include <memory>
#include <Unknwn.h>

/** Utility class that allows creating instances on its own single threaded apartment */
class ComFactory final
{
public:
    ComFactory();

    /** Destructor disconnects all proxies from their stubs. After the apartment is
     * destroyed, calling any functions on the objects created on the apartment will fail */
    ~ComFactory();

    /** Create an instance of a COM object on the apartment. The returned object is a proxy that
     * communicates with a corresponding stub on the apartment. */
    HRESULT CreateInstance(const IID& rclsid, IUnknown* pUnkOuter, const IID& riid, void** ppv);

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};
