#pragma once

#include "Programmer.g.h"

namespace winrt::WinrtServer::implementation
{
    struct Programmer : ProgrammerT<Programmer>
    {
        Programmer() = default;

        void GiveCoffee();
        void WriteDocumentation();
        int Motivation();


    private:
        int m_motivation = 0;
    };
}

namespace winrt::WinrtServer::factory_implementation
{
    struct Programmer : ProgrammerT<Programmer, implementation::Programmer>
    {
    };
}
