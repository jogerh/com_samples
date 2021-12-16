#include "../pch.h"
#include <gtest/gtest.h>
#include <ComUtility/Utility.h>
#include <winrt/WinrtServer.h>

#pragma comment (lib, "runtimeobject.lib")

TEST(WinrtServerTests, RequireThat_ActivateInstance_CreatesProgrammer)
{
    init_apartment(winrt::apartment_type::single_threaded);

    const auto factory = winrt::get_activation_factory(L"WinrtServer.Programmer");
    EXPECT_NO_THROW(factory.ActivateInstance<winrt::WinrtServer::Programmer>());
}

TEST(WinrtServerTests, RequireThat_GivingProgrammerCoffee_IncreasesMotviation)
{
    init_apartment(winrt::apartment_type::single_threaded);

    const auto factory = winrt::get_activation_factory(L"WinrtServer.Programmer");
    const auto programmer = factory.ActivateInstance<winrt::WinrtServer::Programmer>();

    const int motivationBeforeCoffee = programmer.Motivation();

    programmer.GiveCoffee();

    EXPECT_TRUE(programmer.Motivation() > motivationBeforeCoffee);
}

TEST(WinrtServerTests, RequireThat_ProgrammerCanAdd3dCoordinates)
{
    init_apartment(winrt::apartment_type::single_threaded);

    const auto factory = winrt::get_activation_factory(L"WinrtServer.Programmer");
    const auto programmer = factory.ActivateInstance<winrt::WinrtServer::Programmer>();

    winrt::WinrtServer::Pos3 a = { 1, 2, 3 };
    winrt::WinrtServer::Pos3 b = { 1, 2, 3 };

    const auto sum = programmer.Add(a, b);

    EXPECT_EQ(sum.x, a.x + b.x);
    EXPECT_EQ(sum.y, a.y + b.y);
    EXPECT_EQ(sum.z, a.z + b.z);
}

TEST(WinrtServerTests, RequireThat_GetFavorites_ReturnsStructsOfStrings)
{
    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
    HR(initialize);

    ComPtr<ABI::WinrtServer::IProgrammer> programmer;
    HR(ActivateInstance(HStringReference(L"WinrtServer.Programmer").Get(), programmer.GetAddressOf()));

    ABI::WinrtServer::Favorites favorites{};
    HR(programmer->GetFavorites(&favorites));

    // It is the responsible of the caller to ensure that resources that the server
    // allocated to proide output data are released. When working with structs,
    // this can easily cause memory leaks. These are avoided by taking ownership
    // of each struct member
    HString favoriteDrink;
    favoriteDrink.Attach(favorites.Drink);

    HString favoriteActivity;
    favoriteActivity.Attach(favorites.Activity);

    EXPECT_STREQ(favoriteDrink.GetRawBuffer(nullptr), L"Coffee");
    EXPECT_STREQ(favoriteActivity.GetRawBuffer(nullptr), L"Coding");
}