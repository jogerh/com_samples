#pragma once
#include "ThreadSafeQueue.h"
#include <functional>
#include <future>
#include <wrl/wrappers/corewrappers.h>

using Event = Microsoft::WRL::Wrappers::Event;
class ApartmentContext;

/** Utility class that allows executing functions in its own thread/apartment.
 * This models the active object design pattern. */
class ComApartment final
{
public:
    ComApartment();

    /** Destructor disconnects all proxies from their stubs. After the apartment is
     * destroyed, calling any functions on the objects created on the apartment will fail */
    ~ComApartment();

    /** Executes function objects on the apartment. Typically, such function objects will
     * create COM objects. */
    std::future<HRESULT> Invoke(std::function<HRESULT()> callable);

private:
    std::future<HRESULT> InvokeOnApartment(std::function<HRESULT()> callable);
    
    void RunMessagePump();

    std::atomic<DWORD> m_threadId = 0;                          ///< Thread id of the apartment thread
    ThreadSafeQueue<std::packaged_task<HRESULT()>> m_queue;     ///< Queue of tasks to be executed on apartment thread
    std::thread m_thread;                                       ///< The thread that hosts the apartment
    const unsigned int m_newTask;                               ///< Sentinel value used to communicate new tasks to message pump
    std::unique_ptr<ApartmentContext> m_context;                ///< An 'apartment' inside the apartment created by CoInitialize to disconnect proxy/stubs during destruction
    Event m_apartmentInitialized;                               ///< Signals that message pump has started and is ready to receive requests
    Event m_apartmentIsClosed;                                  ///< Signals to the calling thread that the apartment thread is done, and it is safe to join the thread.
};

