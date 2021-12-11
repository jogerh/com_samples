#include "pch.h"

#include <atlbase.h>
#include <gtest/gtest.h>
#include <combaseapi.h>
#include <ComUtility/Utility.h>

using namespace testing;

CComModule s_module; ///< Needed by ATL to make the ATL machinery work for COM

/** Initializes the the main thread as a single threaded COM apartment */
class GlobalEnvironment final : public Environment {
public:
    ComRuntime m_comRuntime;
};

int main(int argc, char** argv)
{
    InitGoogleTest(&argc, argv);
    AddGlobalTestEnvironment(new GlobalEnvironment);
    return RUN_ALL_TESTS();
}