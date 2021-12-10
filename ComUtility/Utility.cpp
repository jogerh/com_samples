// ComUtility.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "Include/ComUtility/Utility.h"

#include <system_error>

void RaiseSystemError(DWORD error, const char* message)
{
    const std::error_code errorCode{static_cast<int>(error), std::system_category()};
    throw std::system_error(errorCode, message);
}
