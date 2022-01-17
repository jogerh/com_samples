#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/IListener.h>
#include <Interfaces/IDog.h>
#include <Interfaces/IPetShop.h>
#include <ComUtility/Utility.h>
#include <wrl.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>

using Microsoft::WRL::ComPtr;

TEST(ManagedServerTests,
    RequireThat_CoCreateInstance_CreatesManagedServer_WhenCalledWithGuidToManagedServer)
{
    CLSID petShopClsid{};
    HR(CLSIDFromString(L"{5011c315-994d-49b4-b737-03a846f590a0}", &petShopClsid));

    ComPtr<IPetShop> petShop;
    HR(CoCreateInstance(petShopClsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IPetShop), &petShop));
    EXPECT_NE(petShop, nullptr);
}

TEST(ManagedServerTests,
    RequireThat_BuyDog_GivesYouAGoodDog)
{
    CLSID petShopClsid{};
    HR(CLSIDFromString(L"{5011c315-994d-49b4-b737-03a846f590a0}", &petShopClsid));

    ComPtr<IPetShop> petShop;
    HR(CoCreateInstance(petShopClsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IPetShop), &petShop));

    ComPtr<IDog> dog;
    HR(petShop->BuyDog(&dog));

    HR(dog->Sit()); // Good dog
}


TEST(ManagedServerTests,
    RequireThat_GetAddress_ReturnsShopAddress)
{
    CLSID petShopClsid{};
    HR(CLSIDFromString(L"{5011c315-994d-49b4-b737-03a846f590a0}", &petShopClsid));

    ComPtr<IPetShop> petShop;
    HR(CoCreateInstance(petShopClsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IPetShop), &petShop));

    Address address;

    HR(petShop->GetAddress(&address));

    EXPECT_STREQ(address.Street, L"Suhms gate");
    EXPECT_STREQ(address.PostalCode, L"0363");
    EXPECT_STREQ(address.City, L"Oslo");

    // With COM, the caller is responsible for releasing resources from getters.
    // This becomes quite inconvenient with structs of strings, since we can't make generic
    // RAII wrappers for structs.
    SysFreeString(address.Street);
    SysFreeString(address.PostalCode);
    SysFreeString(address.City);
}

// Base class to listen events from COM event source
template<unsigned Id, typename Class, typename Events>
struct ListenerImpl :
    IDispEventSimpleImpl<Id, Class, &__uuidof(Events)>,
    IListener
{
    // Connect to toListen event source
    HRESULT Start(IUnknown* toListen) override
    {
        m_toListen = toListen;
        // Setup the connection with the event source
        DispEventAdvise(m_toListen);

        return S_OK;
    }

    // Disconnect from event source
    HRESULT Stop() override
    {
        // Break the connection with the event source
        DispEventUnadvise(m_toListen);

        // Release the application
        m_toListen.Release();

        return S_OK;
    }

protected:
    CComPtr<IUnknown> m_toListen;
};
