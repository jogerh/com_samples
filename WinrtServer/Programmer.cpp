#include "pch.h"
#include "Programmer.h"
#include "Programmer.g.cpp"

namespace winrt::WinrtServer::implementation
{
    int32_t Programmer::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void Programmer::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
