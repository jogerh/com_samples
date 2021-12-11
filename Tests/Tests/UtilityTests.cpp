#include <ComUtility/Utility.h>
#include <gtest/gtest.h>

TEST(ComUtilityTest,
    RequireThat_RaiseSystemError_ThrowsSystemError)
{
    EXPECT_THROW(RaiseSystemError(5, "Some failure"), std::system_error);
}