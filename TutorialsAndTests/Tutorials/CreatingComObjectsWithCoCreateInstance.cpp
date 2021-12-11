#include "../pch.h"
#include <gtest/gtest.h>
#include <Interfaces/IDog.h>
#include <AtlFreeServer/GuardDog.h>
#include <ComUtility/Utility.h>
#include <wrl.h>

using namespace testing;

using Microsoft::WRL::ComPtr;

// 
// This example shows how to create a COM object in C++. 
//
TEST(CreatingComObjects,
    example_using_cocreateinstance_to_create_com_objects)
{

    // Prefer using smart pointers like ATL CComPtr, WRL's ComPtr or similar when managing lifetime of COM objects
    ComPtr<IDog> dog;

    // CoCreateInstance is a fundamental function in the COM runtime, and is creating new instances
    // of COM classes. Here, we use it to create a guard dog object. We can see this because the first
    // argument is the guid of the GuardDog class. This is most conveniently done by using the __uuidof 
    // language extension.
    //
    // The second argument is a pointer to the owning object. This is used if the COM object we create
    // is going to be part of an 'aggregate' COM object. Since we are not creating an aggregate here,
    // we pass in nullptr
    //
    // Then we can decide where we want the object to reside. We can create COM objects in the current
    // process, as a separate process on this machine, or as a process on a different computer. Here we
    // create the object in the current process, since we pass in CLSCTX_INPROC_SERVER.
    // 
    // Since COM classes can implement more than one interface, we also have to specify which interface
    // we want to view the COM object through. We can always request other interfaces from the created
    // object later using QueryInterface. Here we want the IDog interface of GuardDog.
    //
    // Finally, we pass in a pointer that will receive the created guard dog object. Since CoCreateInstance
    // only accepts void pointers, we have to do the somewhat ugly reinterpret_cast. But this is just how
    // it is.

    HR(CoCreateInstance(
        __uuidof(GuardDog),                             // guid that specifies which object we want to create
        nullptr,                                        // optional owning COM objects for aggregation
        CLSCTX_INPROC_SERVER,                           // create object in current process
        __uuidof(IDog),                                 // the interface we want from the created object
        reinterpret_cast<void**>(dog.GetAddressOf()))); // output argument with the created object 

    EXPECT_NE(dog, nullptr);
}


//
// How doew CoCreateInstance work?
//
TEST(CreatingComObjects,
    how_does_cocreateinstance_work)
{
    ComPtr<IDog> dog;

    // Start by putting a breakpoint in DllGetClassObject function in AtlFreeServer/GuardDog.cpp, 
    // and then run this test.
    // 
    // The first thing CoCreateInstance does is to look up the the guid from the first argument in
    // registry (HKLM Software\\Classes\\CLSID\\{<guard dog guid>}\\InprocServer32) to figure out 
    // which dll to load. Then, the COM runtime loads this dll, and calls one of the four COM 
    // functions a COM ddl can have, namely DllGetClassObject.
    // 
    // The COM runtime next calls DllGetClassObject to obtain a factory object that can create objects
    // of GuardDog type. This class factory is implemented by the PuppyFarm class inside AtlFreeServer/
    // GuardDog.cpp.
    // 
    // Once the class factory is obtained, the ComRuntime will call the IClassFactory::CreateInstance
    // function to create the real instances of the GuardDog class.
    // 
    // Finally, it can return the pointer of the new object back to the caller.
    HR(CoCreateInstance(
        __uuidof(GuardDog),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IDog),
        reinterpret_cast<void**>(dog.GetAddressOf())));

    EXPECT_NE(dog, nullptr);
}