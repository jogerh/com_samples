#include "../pch.h"
#include <gtest/gtest.h>
#include <winrt/WinrtServer.h>


TEST(WinrtServerTests, RequireThat_ActivateInstance_CreatesProgrammer)
{
    init_apartment(winrt::apartment_type::single_threaded);

    const auto factory = winrt::get_activation_factory(L"WinrtServer.Programmer");
    EXPECT_NO_THROW(factory.ActivateInstance<winrt::WinrtServer::Programmer>());
}

TEST(WinrtServerTests, RequireThat_GivingProgrammerCoffee_IncreasesMotviation)
{
    init_apartment(winrt::apartment_type::single_threaded);

    winrt::WinrtServer::Programmer programmer; // will trigger WinrtServer.dll loading

    const int motivationBeforeCoffee = programmer.Motivation();

    programmer.GiveCoffee();

    EXPECT_TRUE(programmer.Motivation() > motivationBeforeCoffee);
}

TEST(WinrtServerTests, RequireThat_ProgrammerCanAdd3dCoordinates)
{
    init_apartment(winrt::apartment_type::single_threaded);

    winrt::WinrtServer::Programmer programmer; // will trigger WinrtServer.dll loading

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

    winrt::WinrtServer::Programmer programmer; // will trigger WinrtServer.dll loading

    auto favorites = programmer.GetFavorites();

    EXPECT_EQ(favorites.Activity, L"Coding");
    EXPECT_EQ(favorites.Drink, L"Coffee");
}

TEST(WinrtServerTests, RequireThat_Buffer_ReturnsCorrectValues)
{
    init_apartment(winrt::apartment_type::single_threaded);

    winrt::WinrtServer::Programmer programmer; // will trigger WinrtServer.dll loading

    auto buffer = programmer.Buffer();

    EXPECT_EQ(buffer.size(), 8u);
    EXPECT_EQ(buffer[0], 1);
}