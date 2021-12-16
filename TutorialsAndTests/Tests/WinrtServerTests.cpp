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

TEST(WinrtServerTests, RequireThat_GetFavorites_ReturnsStructWithStrings)
{
    init_apartment(winrt::apartment_type::single_threaded);

    const auto factory = winrt::get_activation_factory(L"WinrtServer.Programmer");
    const auto programmer = factory.ActivateInstance<winrt::WinrtServer::Programmer>();

    auto favorites = programmer.GetFavorites();

    EXPECT_EQ(favorites.Activity, L"Coding");
    EXPECT_EQ(favorites.Drink, L"Coffee");
}