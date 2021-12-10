#pragma once
#include <mutex>
#include <deque>

template <typename T>
class ThreadSafeQueue
{
public:
    void push_back(T elem)
    {
        std::lock_guard guard(m_mutex);
        m_q.push_back(std::move(elem));
    }

    T pop_front()
    {
        std::lock_guard guard(m_mutex);
        auto elem = std::move(m_q.front());
        m_q.pop_front();
        return elem;
    }

    /** Revert a newly added element */
    void pop_back()
    {
        std::lock_guard guard(m_mutex);
        m_q.pop_back();
    }

private:
    std::mutex m_mutex;
    std::deque<T> m_q;
};
