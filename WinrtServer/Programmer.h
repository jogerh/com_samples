#pragma once

#include "Programmer.g.h"

namespace winrt::WinrtServer::implementation
{
    struct Programmer : ProgrammerT<Programmer>
    {
        Programmer();

        void GiveCoffee();
        void WriteDocumentation();
        int Motivation();
        Pos3 Add(Pos3 a, Pos3 b);
        Favorites GetFavorites();
        com_array<uint8_t> Buffer();

    private:
        int m_motivation = 0;
        std::vector<uint8_t> m_buffer;
    };
}

namespace winrt::WinrtServer::factory_implementation
{
    struct Programmer : ProgrammerT<Programmer, implementation::Programmer>
    {
    };
}
