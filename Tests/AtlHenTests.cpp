#include "pch.h"
#include <gtest/gtest.h>
#include <atlcomcli.h>
#include <Interfaces/IHen.h>
#include <Interfaces/IHen_i.c>
#include <AtlHenLib/AtlHenLib.h>
#include <AtlHenLib/AtlHenLib_i.c>
#include <ComUtility/Utility.h>
#include <winrt/base.h>
#include "Mocks/IHenMock.h"

using namespace testing;

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
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Invoke([mainThreadId]
        {
            EXPECT_EQ(GetCurrentThreadId(), mainThreadId);
            return S_OK; // Feel free to put a breakpoint here, and see which thread we are called on
        }));

    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    // To play nice with COM, we should not pass COM interface pointers to other threads
    // unless they are 'agile'. We can do this with a simple wrapper that converts the interfaces
    // to agile interfaces. We could also marshal the interface to a stream using CoMarshalInterThreadInterfaceInStream
    // and CoGetInterfaceAndReleaseStream.
    const auto agileHen = AgilePtr<IHen>(hen);
    const auto agileObserver = AgilePtr<IAsyncCluckObserver>(observer);

    std::thread thread{ [agileHen, agileObserver, mainThreadId]
    {
        ComRuntime runtime{Apartment::MultiThreaded};

        SetThreadDescription(GetCurrentThread(), L"Worker thread"); // Help debugging
        HR(agileHen.Get()->CluckAsync(agileObserver.Get()));

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

// Test that demonstrates how we can stuck with message pump and AgilePtr
// Disabled, just shows where we stuck.
TEST(AtlHenTests, DISABLED_RequireThat_DeadlockIsOccurred_WhenPumpStopsBeforeReleasingComObject)
{
    const auto mainThreadId = GetCurrentThreadId();

    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Invoke([mainThreadId]
        {
            EXPECT_EQ(GetCurrentThreadId(), mainThreadId);
            return S_OK; // Feel free to put a breakpoint here, and see which thread we are called on
        }));

    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    // What will happen if we move agileHen to thread function?
    auto agileHen = AgilePtr<IHen>(hen);
    const auto agileObserver = AgilePtr<IAsyncCluckObserver>(observer);

    // On the line below we move agileHen into lambda, so agileHen from above will be in
    // the next state, AgilePtr::m_agileRef will own nullptr, because agileHen lives in lambda now.
    std::thread thread{ [agileHen = std::move(agileHen), agileObserver, mainThreadId]
    {
        ComRuntime runtime{Apartment::MultiThreaded};

        SetThreadDescription(GetCurrentThread(), L"Worker thread"); // Help debugging
        HR(agileHen.Get()->CluckAsync(agileObserver.Get()));

        // Notify main thread that it can now exit its message loop
        PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
        // What is going on after this line?
        // It is well-known that lambda is a functional object, anonymous structure with 
        // fields from capture list. Well, after this line destructor of captured objects
        // called, but agileHen was moved, and we only one owner, so CComPtr<IAgileReference>::Release()
        // Not only decrement counter, but also will destroy object. But where? On which apparent? 
        // Exactly, on main, but it is impossible because we post exit from message loop.
    } };

    // Pump messages from COM runtime to allow function calls from worker thread to
    // main thread to be processed. This is how COM allows communication between threads.
    MSG message{};
    while (const auto result = GetMessage(&message, 0, 0, 0))
    {
        if (-1 != result)
            DispatchMessage(&message);
    }

    // This join locks with CComPtr<IAgileReference>::Release() what tries to destroy object on this apartment.
    thread.join();
}

// Post WM_QUIT on object destruction
struct ScopedWmQuit
{
    ScopedWmQuit() : m_threadId(GetCurrentThreadId()) { }

    ScopedWmQuit(const ScopedWmQuit&) = delete;
    ScopedWmQuit& operator=(const ScopedWmQuit&) = delete;
    ScopedWmQuit(ScopedWmQuit&&) = delete;
    ScopedWmQuit& operator=(ScopedWmQuit&&) = delete;

    ~ScopedWmQuit()
    {
        PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
    }
private:
    DWORD m_threadId = 0;
};

// Ensure that WM_QUIT posted after m_callable destroyed, to avoid deadlocks (see test above)
template <typename Callable>
struct GuardedCallable
{
    explicit GuardedCallable(Callable callable) : m_callable(std::move(callable)) { }

    void operator()() const
    {
        m_callable();
    }
private:
    // Order is valuable here, ScopedWmQuit created first => destroyed last.
    // unique_ptr used to avoid PostThreadMessage on moved ScopedWmQuit, because it is simpler
    std::unique_ptr<ScopedWmQuit> m_wmQuit = std::make_unique<ScopedWmQuit>();
    Callable m_callable;
};

// Test that demonstrates how to handle situations with move
TEST(AtlHenTests, RequireThat_AgilePtrMove_DoesNotLeadToDeadlock_WhenPumpStopsAfterReleasingComObject)
{
    const auto mainThreadId = GetCurrentThreadId();

    auto observer = make_self<IAsyncCluckObserverMock>();
    EXPECT_CALL(*observer, OnCluck()).WillOnce(Invoke([mainThreadId]
        {
            EXPECT_EQ(GetCurrentThreadId(), mainThreadId);
            return S_OK; // Feel free to put a breakpoint here, and see which thread we are called on
        }));

    CComPtr<IHen> hen;
    HR(CoCreateInstance(CLSID_AtlHen, nullptr, CLSCTX_INPROC_SERVER, IID_IHen, reinterpret_cast<void**>(&hen)));

    auto agileHen = AgilePtr<IHen>(hen);
    const auto agileObserver = AgilePtr<IAsyncCluckObserver>(observer);

    // The lambda function below is very greedy. It personally owns agileHen, and as a consequence destroys it.
    // Destruction will access to the main thread, COM does it via Windows Messages.
    // So GuardedCallable is created to ensure that post WM_QUIT was called after functor destruction.
    // This will allow us to destroy agileHen owned IAgileReference on main thread apartment.

    // Attentive programmer would ask why don't we use order of lambda capture? The answer is simple, it is not defined.
    // https://stackoverflow.com/questions/12520611/c11-in-what-order-are-lambda-captures-destructed
    std::thread thread{ GuardedCallable([agileHen = std::move(agileHen), agileObserver, mainThreadId]
    {
        ComRuntime runtime{Apartment::MultiThreaded};

        SetThreadDescription(GetCurrentThread(), L"Worker thread"); // Help debugging
        HR(agileHen.Get()->CluckAsync(agileObserver.Get()));
    })};

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
