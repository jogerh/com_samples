#include <ComUtility/ComFactory.h>
#include <ComUtility/Utility.h>
#include <Interfaces/IHen.h>
#include <AtlServer/AtlServer.h>
#include <gtest/gtest.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

TEST(ComApartmentTests,
    RequireThat_CreateInstance_CreatesInstance)
{
    ComFactory factory;
    ComPtr<IHen> hen;

    EXPECT_HRESULT_SUCCEEDED(factory.CreateInstance(__uuidof(AtlHen), nullptr, __uuidof(IHen), reinterpret_cast<void**>(hen.GetAddressOf())));

    HR(hen->Cluck());
}

TEST(ComApartmentTests,
    RequireThat_Destructor_DisconnectsProxy)
{
    
    ComPtr<IHen> hen;
    {
        ComFactory factory;
        ASSERT_HRESULT_SUCCEEDED(factory.CreateInstance(__uuidof(AtlHen), nullptr, __uuidof(IHen), reinterpret_cast<void**>(hen.GetAddressOf())));
    }

    EXPECT_EQ(RPC_E_SERVER_DIED_DNE, hen->Cluck());
}