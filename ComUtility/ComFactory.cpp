#include "pch.h"

#include "Include/ComUtility/ComFactory.h"
#include "ComApartment.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

struct ComFactory::impl
{
    ComApartment m_apartment;
};

ComFactory::ComFactory()
    : m_impl{std::make_unique<impl>()}
{
}

ComFactory::~ComFactory()
{
}

HRESULT ComFactory::CreateInstance(const IID& rclsid, IUnknown* pUnkOuter, const IID& riid, void** ppv)
{
    // This stream will contain the marshaled interface to the created object
    ComPtr<IStream> stream = nullptr;

    // Delegate construction to the apartment, to create the object on a separate thread
    const auto result = m_impl->m_apartment.Invoke([rclsid, pUnkOuter, &stream]()
    {
        ComPtr<IUnknown> punk;
        const auto result = CoCreateInstance(
            rclsid,
            pUnkOuter,
            CLSCTX_INPROC_SERVER,
            IID_IUnknown,
            reinterpret_cast<void**>(punk.GetAddressOf()));

        if (result != S_OK)
            return result;

        // Marshal interface to the stream. This allows unmarshaling the interface
        // in a different thread
        return CoMarshalInterThreadInterfaceInStream(IID_IUnknown, punk.Get(), stream.GetAddressOf());
    }).get();

    if (result != S_OK)
        return result;

    // Get the interface marshaled onto the calling thread
    return CoGetInterfaceAndReleaseStream(stream.Detach(), riid, ppv);
}
