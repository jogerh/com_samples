#pragma once
#include <Interfaces/IPostman.h>
#include <gmock/gmock.h>
#include <wrl/implements.h>
namespace wrl = Microsoft::WRL;

struct IPostmanMock : wrl::RuntimeClass<wrl::RuntimeClassFlags<wrl::ClassicCom>, IPostman>
{
    MOCK_METHOD(HRESULT, OnBitten, (), (override));
};