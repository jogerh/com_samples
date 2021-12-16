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

    // Obtain the factory that can create programmers
    ComPtr<IActivationFactory> programmerFactory;
    HR(GetActivationFactory(HStringReference(L"WinrtServer.Programmer").Get(), &programmerFactory));

    // Use the factory to create a programmer
    ComPtr<IInspectable> programmer;
    HR(programmerFactory->ActivateInstance(programmer.GetAddressOf()));

    CComPtr<ABI::WinrtServer::IProgrammer> p;
    HR(programmer->QueryInterface(__uuidof(ABI::WinrtServer::IProgrammer), reinterpret_cast<void**>(&p)));

    INT32 v = 0;
    EXPECT_EQ(p->get_MyProperty(&v), E_NOTIMPL);
}