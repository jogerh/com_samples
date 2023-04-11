#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/IDog.h>
#include <AtlFreeServer/GuardDog.h>
#include <ComUtility/Utility.h>
#include <winrt/base.h>
#include <wrl.h>
#include "Mocks/IPostmanMock.h"

using namespace testing;
using Microsoft::WRL::ComPtr;

// Test that demonstrates how to create a COM object based upon its class id
TEST(AtlFreServerTests, RequireThat_CoCreateInstance_CreatesGuardDog_WhenCalledWithGuardDogUid)
{
    ComPtr<IDog> dog;
    HR(CoCreateInstance(__uuidof(GuardDog), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IDog), &dog));
    EXPECT_NE(dog, nullptr); 
}

// Test that demonstrates how we can use mocks with COM by creating local instances
TEST(AtlFreServerTests, RequireThat_Bite_CausesDogToBitePostman)
{
    const auto postman = wrl::Make<IPostmanMock>();
    EXPECT_CALL(*postman.Get(), OnBitten()).WillOnce(Invoke([] {
        return S_OK;
    }));

    ComPtr<IDog> guardDog;
    HR(CoCreateInstance(__uuidof(GuardDog), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IDog), &guardDog));

    HR(guardDog->Bite(postman.Get()));
}
