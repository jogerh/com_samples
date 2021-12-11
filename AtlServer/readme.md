# AtlServer

This library implements a COM server using ATL. It shows the various parts of how to create a COM dll that supports in-process and out-of-process activation.

## .idl files - Interface definitions

.idl files contains COM interface definitions, library definitions, and class definitions. They are written using the Interface Definition Language (IDL) and are compiled by the midl compiler. Compilation of the .idl files happens before the C/C++ compiler is invoked. The output of the midl compiler is files containing C/C++ code that represent the same interfaces, just in C/C++. These generated files can then be used inside the C++ implementation of the COM dll to implement COM classes and COM libraries.

When building large applications, it can often be useful to separate .idl files containing pure interface definitions from .idl files containing library and class definitions. This practice helps preventing unnecessary build dependencies between projects for building dlls and executables.

## .rc files - Resource files

.rc files describes which resources should be embedded into the COM dll. Resources can be translation strings, copy right information, icon bitmaps, registry scripts or type libraries.

The .rc resource files are parsed by the resource compiler and outputs a .res file that is linked into the dll by the linker. When building a COM dll with ATL, the .rc file should contain references to the registry scripts (.rgs files), and to the type library (.tlb file). This way, the registry scripts and type library gets embedded as resources into the dll. 

Visual Studio project wizards creates the skeleton .rc files when creating a new project, but it is often necessary to manually edit them to add new functionality, unless all new classes are added using Visual Studio wizards.

## Resource.h file

The Resource.h is used to bind C++ code that depend on resources to the individual resources in the .rc file. This is done through constants that identifies individual resources. For example, the type library may be defined by the constant 

## .rgs files - Registry scripts

.rgs files are scripts containing instructions for the ATL Registry Component (Registrar) on how to update the registry when registering a COM dll. These files are not interpreted at compile time. Instead, they are embedded inside the dll as resources if they are referenced in the .rc file.

When registering the dll with regsvr32, regsvr32 will call the DllRegisterServer function of the COM dll. Since we use ATL for this server, the DllRegisterServer function will call the ATL function s_atlModule.DllRegisterServer(), which load resources from the dll. Once an .rgs resource is found, the ATL Registry Component (Registrar) embedded in the dll will parse the script and call the native Windows API registry functions to update the registry.

.rgs files can be created using the Visual Studio code wizard, but unfortunately these wizards fail to add the 'appid' entry which is needed for running the dll out of process through the dllhost.exe process. In the end, it is usually better to write the .rgs files by copying from working code.

## .def file - Module definition file

The module definition file tells the linker which functions to export from the dll. The COM standard defines a set of four functions that all COM dlls have to export. 

* *DllGetClassObject*: Called by the COM runtime to create instances (COM objects) from the COM dll.
* *DllCanUnloadNow*: Called by the COM runtime to determine if all references to the COM dll have been released, to see if the dll can be unloaded.
* *DllRegisterServer*: Called by for example by regsvr32 to install/register the COM dll. This will allow the dll to update the registry with the identity of the COM classes and interfaces that it provides, along with any type libraries.
* *DllUnregisterServer*: Called by for example regsv32 to uninstall/unregister the COM dll. This is used to clean up the registry for COM dlls that are being uninstalled.

With COM, we never have to link to the COM dll, so we don't need to export the module name, and we can export the functions as 'private'. 

## .tlb files - COM Type libraries

.tlb files are binary files created by the midl compiler, and contains the same information that the input .idl files contained. This includes information about the data types (enums, structs, unions), interfaces, libraries, COM classes, and also references to other type libraries. This .tlb is embedded into the COM dll by the linker through the .rc file. 

The purpose of the type library is to have a runtime description of all the types that the COM dll exposes, and is necessary when using the COM dll from other programming languages than C/C++. Code written in C/C++ do not depend on the type libraries, because the binary format of the COM standard matches a subset of C/C++. Still, the type library may be needed when building applications that uses 'remoting', where objects have to be serialized/marshaled between threads or processes.

The type library is also essential when using the IDispatch interfaces. IDispatch interfaces allow consuming the COM component from scripting languages, where runtime type information is needed to determine which functions the interfaces expose, and which argument they accepts.

The .tlb files are also used when generating .NET interop assemblies using the TlbImp.exe. This allows consuming the COM dll directly from .NET code without use of glue code.