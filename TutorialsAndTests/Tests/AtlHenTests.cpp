#include "../pch.h"
#include <gtest/gtest.h>
#include <atlcomcli.h>
#include <Interfaces/IHen.h>
#include <Interfaces/IHen_i.c>
#include <AtlServer/AtlServer.h>
#include <AtlServer/AtlServer_i.c>
#include <ComUtility/Utility.h>
#include <winrt/base.h>
#include <wrl.h>
#include <future>
#include "Mocks/IHenMock.h"
#include "ComUtility/Utility.h"

using namespace testing;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::AgileRef;

// Test that demonstrates how to create a COM object based upon its class id
TEST(AtlHenTests, RequireThat_CoCreateInstance_CreatesAtlHen_WhenCalledWithHensClassId)
{
    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));
    EXPECT_NE(hen, nullptr);
}

// Test that demonstrates how to create a COM object based upon its class id
TEST(AtlHenTests, RequireThat_CoCreateInstance_CreatesCluckObserver_WhenCalledWithCluckObserversClassId)
{
    CComPtr<IAsyncCluckObserver> observer;
    HR(CoCreateInstance(CLSID_AtlCluckObserver, nullptr, CLSCTX_INPROC_SERVER, IID_IAsyncCluckObserver, reinterpret_cast<void**>(&observer)));
    EXPECT_NE(observer, nullptr);
}

// Test that demonstrates how to send a COM interface as argument to a COM function
TEST(AtlHenTests, RequireThat_CluckAsync_ReturnsSuccess_WhenCalledWithValidCluckObserver) {
    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    CComPtr<IAsyncCluckObserver> cluckObserver;
    HR(CoCreateInstance(CLSID_AtlCluckObserver, nullptr, CLSCTX_INPROC_SERVER, IID_IAsyncCluckObserver, reinterpret_cast<void**>(&cluckObserver)));

    EXPECT_HRESULT_SUCCEEDED(hen->CluckAsync(cluckObserver));
}

// Test that demonstrates how we can use mocks with COM by creating local instances
TEST(AtlHenTests, RequireThat_Cluck_IsCalledOnAsyncCluckObserver_WhenPassedToHen)
{
    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Return(S_OK));

    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    HR(hen->CluckAsync(observer));
}

// Test that demonstrates that even if the COM server runs as a separate process, we can still pass it
// interfaces to local objects that are not exposed to the COM runtime through registry.
TEST(AtlHenTests, RequireThat_Cluck_IsCalledOnAsyncCluckObserver_WhenPassedToHenThatLivesInSeparateProcess)
{
    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Return(S_OK));

    // Create the hen on a separate process running through dllhost. You can find the dllhost instance
    // in task manager by looking for the 860d9d2c-315d-4e01-93b6-ded0e8d133c3 in the command line
    // This is the AppId registered in the AtlHenModule.cpp
    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_LOCAL_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    HR(hen->CluckAsync(observer));
}

// Test that demonstrates how COM marshals calls across apartments
TEST(AtlHenTests, RequireThat_Cluck_IsExecutedOnMainThread_WhenCalledFromWorkerThread)
{
    const auto mainThreadId = GetCurrentThreadId();

    // Configure the async cluck observer to fail unless it is called on the main thread
    // After all, it was created on a single threaded apartment (STA), and it should not be
    // allowed to call it from any other thread.
    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Invoke([mainThreadId] {
        EXPECT_EQ(GetCurrentThreadId(), mainThreadId);
        return S_OK; // Feel free to put a breakpoint here, and see which thread we are called on
    }));

    ComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, &hen));

    // To play nice with COM, we should not pass COM interface pointers to other threads
    // unless they are 'agile'. We can do this with a simple wrapper that converts the interfaces
    // to agile interfaces. We could also marshal the interface to a stream using CoMarshalInterThreadInterfaceInStream
    // and CoGetInterfaceAndReleaseStream. Also use AgileRef from WRL to show how it can be used for marshaling.
    AgileRef agileHen;
    // AsAgile also use RoGetAgileReference.
    HR(hen.AsAgile(&agileHen));
    const auto agileObserver = AgilePtr<IAsyncCluckObserver>(observer);

    std::thread thread{ [agileHen, agileObserver, mainThreadId]
    {
        ComRuntime runtime{Apartment::MultiThreaded};

        SetThreadDescription(GetCurrentThread(), L"Worker thread"); // Help debugging
        ComPtr<IHen> henLocal;
        // All methods in AgileRef calls Resolve to get pointer what is safe to use in this thread.
        HR(agileHen.As(&henLocal));
        HR(henLocal->CluckAsync(agileObserver.Get()));
        // Manually reset ComPtr to avoid deadlock related to destruction after
        // PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
        // henLocal is a proxy object created by COM runtime, and destruction
        // of this object marshalled to the main thread, for more details see
        // https://docs.microsoft.com/en-us/windows/win32/com/proxy
        henLocal.Reset();

        // Notify main thread that it can now exit its message loop
        PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
    } };

    // Pump messages from COM runtime to allow function calls from worker thread to
    // main thread to be processed. This is how COM allows communication between threads.
    MSG message{};
    while (const auto result = GetMessage(&message, 0, 0, 0))
    {
        if (-1 != result)
            DispatchMessage(&message);
    }

    thread.join();
}

// Test that demonstrates what happens when calling an apartment threaded object from a worker thread
TEST(AtlHenTests, ObserveThat_CluckAsync_Fails_WhenCalledFromWorkerThread)
{
    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).Times(Exactly(0));

    // Create the hen on a separate process running through dllhost. You can find the dllhost instance
    // in task manager by looking for the 860d9d2c-315d-4e01-93b6-ded0e8d133c3 in the command line
    // This is the AppId registered in the AtlHenModule.cpp
    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_LOCAL_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    auto result = std::async(std::launch::async, [&] {
        ComRuntime comRuntime{ Apartment::MultiThreaded };
        return hen->CluckAsync(observer);
    });

    EXPECT_EQ(result.get(), RPC_E_WRONG_THREAD);
}

// Test that demonstrates use of the free threaded marshaler which allows calling an object from any thread.
TEST(FreeThreadedHenTests, RequireThat_Cluck_IsCalledOnAsyncCluckObserver_WhenCalledFromWorkerThread)
{
    auto observer = winrt::make_self<FreeThreadedCluckObserver>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Invoke([] {
        return S_OK;
    }));

    // Create the hen on a separate process running through dllhost. You can find the dllhost instance
    // in task manager by looking for the 860d9d2c-315d-4e01-93b6-ded0e8d133c3 in the command line
    // This is the AppId registered in the AtlHenModule.cpp
    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_FreeThreadedHen, nullptr, CLSCTX_LOCAL_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    auto result = std::async(std::launch::async, [&] {
        ComRuntime comRuntime{Apartment::MultiThreaded};
        return hen->CluckAsync(observer.get());
    });

    EXPECT_EQ(result.get(), S_OK);
}