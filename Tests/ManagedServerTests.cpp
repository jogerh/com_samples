#include "pch.h"
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