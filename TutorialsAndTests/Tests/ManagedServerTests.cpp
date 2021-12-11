#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/IDog.h>
#include <Interfaces/IPetShop.h>
#include <ComUtility/Utility.h>
#include <wrl.h>

using namespace testing;
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