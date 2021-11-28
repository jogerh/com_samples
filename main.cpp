#include "ComServerModule.h"
#include "ComServerModule_i.c"
#include "Interfaces_i.c"
#include <atlcomcli.h>
int main() {
    CComPtr<IComServer> o;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    auto hr = CoCreateInstance(CLSID_ComServer, nullptr, CLSCTX_INPROC_SERVER, IID_IComServer, reinterpret_cast<void**>(&o));
    return 0;
}