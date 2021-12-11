# Interfaces

This project contains the interfaces used by most COM examples in this repository. By keeping the interfaces separated from the implementations, we avoid unnecessary build dependencies between implementation projects. 

The interfaces are written using the Interface Definition Language (IDL), and are compiled by the midl.exe compiler. The Interface Definition Language can only be used to define interfaces, and favors separating interfaces from implementations. This allows interfaces to be completely language agnostic because it is impossible to leak binary implementation details through the interface.

## midl compiler outputs

During midl compilation, each .idl file produces:

* A header file (.h) containing the interface definitions. With C++, these interfaces are pure virtual interfaces, and are a one-to-one match with the interface defined in the .idl file.
* A identifier file  (_i.c) containing Interface Identifiers (IIDs) and optionally Class Identifiers (CLSIDs).
* Optionally a proxy stub file (_p.c) containing an implementation of the proxy/stub for the interfaces. The proxy/stub code can be used to allow COM objects to communicate across processes and threads.
* Optionally a type library file (.tlb) if the .idl file contains a 'library' section.

The Interfaces.idl is different from the other .idl files because it also provides a 'library' section. A 'library' is a collection of interfaces, COM class definitions, and COM types. When the midl.exe comes over a library, it generates a .tlb file that contains a type library. The .tlb file contains the same information as the .idl file, but in binary form. The type library is discussed in more detail in [AtlServer](../AtlServer/readme.md#tlb-files---com-type-libraries).

To add COM types to the type library, simply add forward references to the interface or type from the 'library' section.

## The interop assembly

The Interfaces project also produces a interop assembly (.dll file) through its post build step. This interop assembly is generated from the Type Library (.tlb) using the TlbImp.exe utility. 

    TlbImp.exe /nologo /machine:$(PlatformTarget) $(OutDir)\$(TargetName).tlb /out:$(OutDir)$(TargetName)Interop.dll

The output from TlbImp.exe is an interop dll. The interop dll contains the same type information as the .tlb file, but can be consumed by .NET projects. When a .NET project references an interop dll, the interfaces and other COM types become available as regular .NET types in the .NET client code. .NET interop is demonstrated in [InteropTests](../InteropTests)