# AtlFreeServer

This library implements a COM server without use of ATL and type libraries. It shows how to implement registration using a table based approach instead of the ATL Registry Component (Registrar). To support remoting without a type library, we use MIDL generated proxy/stubs and this server merges the proxy into the COM dll. 

This means that we can create a COM server without use of .rc files, .rgs files, .tlb files and embedded resources. See [AtlServer](../AtlServer/) for details.

From the registration code, we can see that COM classes are registered under

    HKEY_LOCAL_MACHINE/Software/Classes/CLSID/{Class UID}

COM Interfaces are registered under

    HKEY_LOCAL_MACHINE/Software/Classes/Interface/{Interface UID}

To make remoting extra exciting, we choose a 'Free' ThreadingModel for this COM server. For now, we only support in-process activation.

These examples are taken from 'Essentials Of COM Part 2' by Kenny Kerr. See also https://kennykerr.ca/courses/
