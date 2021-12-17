#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/IRoyalPython.h>
#include <ComUtility/Utility.h>
#include <wrl.h>

using namespace testing;
using Microsoft::WRL::ComPtr;

TEST(PyComServerTests,
    RequireThat_CoCreateInstance_CreatesPythonServer_WhenCalledWithGuidToPythonServer)
{
    CLSID classid{};
    HR(CLSIDFromProgID(L"PyComServer.RoyalPython.1", &classid));

    ComPtr<IRoyalPython> py;
    HR(CoCreateInstance(classid, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(IRoyalPython), &py));
    EXPECT_NE(py, nullptr);

    py->Eat();
}