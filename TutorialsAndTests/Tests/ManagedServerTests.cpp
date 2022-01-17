#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/Interfaces.h>
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

// Interface to add call expectations
struct IEventsHandler
{
    virtual ~IEventsHandler() = default;
    virtual void OnOpenedImpl() const = 0;
};

_ATL_FUNC_INFO OnOpenedInfo = { CC_STDCALL, VT_EMPTY, 0 };

// Real PetShopListener, inspired from :
// https://github.com/microsoft/VCSamples/blob/9e1d4475555b76a17a3568369867f1d7b6cc6126/VC2008Samples/ATL/General/ATLEventHandling/Simple.h
struct PetShopListener :
    CComObjectRootEx<CComMultiThreadModel>,
    CComCoClass<PetShopListener>,
    ListenerImpl<1, PetShopListener, _DPetShopEvents>,
    IEventsHandler
{
    BEGIN_COM_MAP(PetShopListener)
        COM_INTERFACE_ENTRY(IListener)
    END_COM_MAP()

    void __stdcall OnOpened()
    {
        OnOpenedImpl();
    }

    MOCK_METHOD(void, OnOpenedImpl, (), (const override));

    BEGIN_SINK_MAP(PetShopListener)
        SINK_ENTRY_INFO(/*nID =*/ 1, __uuidof(_DPetShopEvents), /*dispid =*/ 1, OnOpened, &OnOpenedInfo)
    END_SINK_MAP()
};

TEST(ManagedServerTests,
    RequireThat_Open_TriggersOpenedEvent_WhenCalled)
{
    CLSID petShopClsid{};
    HR(CLSIDFromString(L"{5011c315-994d-49b4-b737-03a846f590a0}", &petShopClsid));

    // Create managed PetShop what can Raise events
    ATL::CComPtr<IPetShop> petShop;
    HR(CoCreateInstance(petShopClsid, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IPetShop), reinterpret_cast<void**>(&petShop)));
    EXPECT_NE(petShop, nullptr);

    // Create local instance for PetShopListener
    auto petShopListener = make_self<PetShopListener>();

    // Subscribe to the petShop events
    petShopListener->Start(petShop);

    // We expect that OnOpenedImpl called while we subscribed
    EXPECT_CALL(*petShopListener, OnOpenedImpl())
        .Times(1);

    // Raise Opened event, this should call PetShopListener::OnOpened
    petShop->Open();

    // Unsubscribe from the petShop events
    petShopListener->Stop();

    // Raise Opened event, but no Listeners and PetShopListener::OnOpened is not called
    petShop->Open();
}
