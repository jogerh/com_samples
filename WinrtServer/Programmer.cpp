#include "pch.h"
#include "Programmer.h"
#include "Programmer.g.cpp"

namespace winrt::WinrtServer::implementation
{
    Programmer::Programmer() {
        m_buffer = { 1, 2, 3, 4, 5, 6, 7, 8 };
    }

    void Programmer::GiveCoffee()
    {
        ++m_motivation;
    }

    void Programmer::WriteDocumentation()
    {
        --m_motivation;
    }

    int Programmer::Motivation()
    {
        return m_motivation;
    }

    Pos3 Programmer::Add(Pos3 a, Pos3 b)
    {
        Pos3 sum = {}; // zero-initialize
        sum.x = a.x + b.x;
        sum.y = a.y + b.y;
        sum.z = a.z + b.z;
        return sum;
    }

    Favorites Programmer::GetFavorites()
    {
        Favorites favorites{};
        favorites.Activity = L"Coding";
        favorites.Drink = L"Coffee";
        return favorites;
    }

    com_array<uint8_t> Programmer::Buffer()
    {
        // return a copy
        return com_array<uint8_t>{ m_buffer.begin(), m_buffer.end() };
    }
}
