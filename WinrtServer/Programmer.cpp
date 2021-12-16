#include "pch.h"
#include "Programmer.h"
#include "Programmer.g.cpp"

namespace winrt::WinrtServer::implementation
{
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

    Favorites Programmer::GetFavorites()
    {
        Favorites favorites{};
        favorites.FavoriteActivity = L"Coding";
        favorites.FavoriteDrink = L"Coffee";

        return favorites;
    }

    Pos3 Programmer::Add(Pos3 a, Pos3 b)
    {
        Pos3 sum = {}; // zero-initialize
        sum.x = a.x + b.x;
        sum.y = a.y + b.y;
        sum.z = a.z + b.z;
        return sum;
    }
}
