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