#pragma once

#include "Programmer.g.h"

namespace winrt::WinrtServer::implementation
{
    struct Programmer : ProgrammerT<Programmer>
    {
        Programmer() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::WinrtServer::factory_implementation
{
    struct Programmer : ProgrammerT<Programmer, implementation::Programmer>
    {
    };
}
