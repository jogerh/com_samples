#include "../pch.h"
#include <gtest/gtest.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <ComUtility/Utility.h>
#include <WinrtServer/WinrtServer.h>

using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#pragma comment (lib, "runtimeobject.lib")

TEST(WinrtServerTests, RequireThat_ActivateInstance_CreatesProgrammer)
{
    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
    HR(initialize);

    ComPtr<ABI::WinrtServer::IProgrammer> programmer;
    HR(ActivateInstance(HStringReference(L"WinrtServer.Programmer").Get(), programmer.GetAddressOf()));

    EXPECT_NE(programmer.Get(), nullptr);
}

TEST(WinrtServerTests, RequireThat_GivingProgrammerCoffee_IncreasesMotviation)
{
    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
    HR(initialize);

    ComPtr<ABI::WinrtServer::IProgrammer> programmer;
    HR(ActivateInstance(HStringReference(L"WinrtServer.Programmer").Get(), programmer.GetAddressOf()));

    int motivationBeforeCoffee = 0;
    HR(programmer->get_Motivation(&motivationBeforeCoffee));

    HR(programmer->GiveCoffee());

    int motivationAfterCoffee = 0;
    HR(programmer->get_Motivation(&motivationAfterCoffee));

    EXPECT_TRUE(motivationAfterCoffee > motivationBeforeCoffee);
}

TEST(WinrtServerTests, RequireThat_ProgrammerCanAdd3dCoordinates)
{
    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
    HR(initialize);

    ComPtr<ABI::WinrtServer::IProgrammer> programmer;
    HR(ActivateInstance(HStringReference(L"WinrtServer.Programmer").Get(), programmer.GetAddressOf()));

    ABI::WinrtServer::Pos3 a = { 1, 2, 3 };
    ABI::WinrtServer::Pos3 b = { 1, 2, 3 };
    ABI::WinrtServer::Pos3 sum = {}; // zero-initialize
    HR(programmer->Add(a, b, &sum));

    EXPECT_EQ(sum.x, a.x + b.x);
    EXPECT_EQ(sum.y, a.y + b.y);
    EXPECT_EQ(sum.z, a.z + b.z);
}