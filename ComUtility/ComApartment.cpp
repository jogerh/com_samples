#include "pch.h"
#include "ComApartment.h"
#include "Include/ComUtility/Utility.h"
#include <cassert>
#include <ctxtcall.h>
#include <wrl.h>


using Microsoft::WRL::ComPtr;

/** An 'apartment' inside the apartment that allows cleaning
    up objects that are left in the apartment before we shut down. */
class ApartmentContext
{
public:
    ApartmentContext()
    {
        HR(CoCreateInstance(CLSID_ContextSwitcher, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_context)));
    }

    /**
     * @brief Execute a callable expression as a context callback.
     *
     * Any objects created in a context lives inside that context.
     * This allows us to disconnect all proxies using CoDisconnectContext.
     *
     * See Raymond Chen 'The Old New Thing How do you get into a context via IContextCallback::ContextCallback?'
     * and https://github.com/microsoft/cppwinrt source code.
     */

    template <typename Callable>
    HRESULT Invoke(Callable&& callable)
    {
        ComCallData data;
        data.pUserDefined = &callable;
        return m_context->ContextCallback(
            [](ComCallData* data)
            {
                auto& callable = *static_cast<Callable*>(data->pUserDefined);
                return callable();
            }, &data, IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr);
        // 'IID_ICallbackWithNoReentrancyToApplicationSTA, 5' means No ASTA reentrancy	
    }

    /** Force all stubs to disconnect from their proxies
    * Effectively, this will release all objects created
    * on this apartment. The ComApartment destructor will
    * be pumping messages to serve these stubs until
    * all have been released. */
    HRESULT Disconnect()
    {
        return Invoke([]
        {
            return CoDisconnectContext(INFINITE);
        });
    }

private:
    ComPtr<IContextCallback> m_context;
};

namespace
{
    Event CreateNonSignaledManualResetEvent()
    {
        Event event{CreateEvent(nullptr, /*manual reset:*/ TRUE, /*not initially signaled:*/ FALSE, nullptr)};
        if (!event.IsValid())
            RaiseSystemError(GetLastError(), "ComApartment failed to create shutdown event");
        return event;
    }

    UINT RegisterTaskMessage(const std::wstring& messageName)
    {
        const auto message = RegisterWindowMessage(messageName.c_str());
        if (message == 0u)
            RaiseSystemError(GetLastError(), "Failed to register window message");
        return message;
    }
}

ComApartment::ComApartment()
    : m_newTask{RegisterTaskMessage(L"ScThread_ComApartment_NewTask")}
      , m_apartmentInitialized{CreateNonSignaledManualResetEvent()}
      , m_apartmentIsClosed{CreateNonSignaledManualResetEvent()}
{
    m_thread = std::thread([this]
    {
        SetThreadDescription(GetCurrentThread(), L"ComApartment"); // Debugging help
        RunMessagePump();
    });

    // Wait until thread has actually started, and the Windows
    // message pump is initialized.
    WaitForSingleObject(m_apartmentInitialized.Get(), INFINITE);

    // Initialize COM and the apartment context on the apartment
    // thread and throw if it fails.
    HR(InvokeOnApartment([this]
    {
        const auto result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        m_context = std::make_unique<ApartmentContext>();
        return result;
    }).get());
}


std::future<HRESULT> ComApartment::Invoke(std::function<HRESULT()> callable)
{
    // Make sure the callable is called within the scope of the apartment context
    // to add a barrier between the bare COM apartment and the stubs that may get
    // created
    return InvokeOnApartment([func = std::move(callable), this]
    {
        return m_context->Invoke([&] { return func(); });
    });
}

std::future<HRESULT> ComApartment::InvokeOnApartment(std::function<HRESULT()> callable)
{
    assert(m_threadId != 0); // To document that at this time, m_threadId is always non-zero.

    std::packaged_task<HRESULT()> task([func = std::move(callable), this]
    {
        return func();
    });

    auto future = task.get_future();
    m_queue.push_back(std::move(task));

    if (PostThreadMessage(m_threadId, m_newTask, 0, 0) == 0)
    {
        // Message queue is likely full. Remove message from queue and throw.
        // This gives strong exception guarantee. Do not pass result through
        // the future, because we want to detect this failure immediately.
        const auto err = GetLastError();
        m_queue.pop_back();
        RaiseSystemError(err, "Failed to execute task");
    }

    return future;
}

ComApartment::~ComApartment()
{
    const auto result = InvokeOnApartment([this]
    {
        // We are shutting down the apartment, but to make sure CoUninitialize
        // can complete, we need to disconnect any remaining proxies to ensure
        // that we do not end up waiting on callbacks to client.
        const auto result = m_context->Disconnect();
        if (result != S_OK)
            return result;

        m_context = nullptr;
        CoUninitialize();
        return S_OK;
    }).get();

    if (result != S_OK)
        std::abort(); // Logically impossible

    // If Windows message queue is full, we need to repeatedly
    // try to post the quit message, hoping that the queue
    // will eventually drain. Anyway, the thread will not join
    // until the WM_QUIT message is received.
    while (PostThreadMessage(m_threadId, WM_QUIT, 0, 0) == 0)
    {
        if (GetLastError() != ERROR_NOT_ENOUGH_QUOTA)
        {
            // If we get here, the thread id was invalid, or
            // the message queue was not created. Both are
            // logically impossible.
            std::abort();
        }

        // Let the message pump get time to process some messages
        Sleep(100);
    }

    // Pump messages while we are waiting for the apartment to shut down.
    // This is needed if an object on the apartment still needs to communicate
    // with the owning apartment (main thread) as part of shutdown
    AtlWaitWithMessageLoop(m_apartmentIsClosed.Get()); // Return value does not matter here.
    m_thread.join();
}

void ComApartment::RunMessagePump()
{
    // This is an idiomatic way of initializing the Windows message queue.
    // PostMessage will fail until the message queue is ready.
    MSG firstMsg{};
    PeekMessage(&firstMsg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    m_threadId = GetCurrentThreadId();

    // Let the caller know we are initialized and ready to go.
    SetEvent(m_apartmentInitialized.Get());

    MSG msg{};
    while (const auto result = GetMessage(&msg, nullptr, 0, 0))
    {
        if (-1 == result)
            RaiseSystemError(GetLastError(), "Failed to get message from queue");

        if (msg.message == m_newTask)
        {
            auto callable = m_queue.pop_front();
            callable();
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (SetEvent(m_apartmentIsClosed.Get()) == 0)
        RaiseSystemError(GetLastError(), "Failed to signal owning thread");
}
