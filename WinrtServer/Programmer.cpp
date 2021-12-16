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
}
