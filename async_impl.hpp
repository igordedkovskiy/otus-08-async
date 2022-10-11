#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "CmdCollector.hpp"

namespace async
{

using size_type = std::size_t;
using handler_t = void*;

class Handlers
{
private:
    using handlers_t = std::unordered_map<handler_t, std::unique_ptr<CmdCollector>>;

public:
    handlers_t::iterator find(handler_t h);

    handlers_t::iterator end();

    handler_t create(std::unique_ptr<CmdCollector> collector);

    void destroy(handler_t h);

    size_type size() const noexcept;

private:
    handlers_t m_handlers;
};


template<typename Q> class QueueSyncContext
{
public:
    template<typename F, typename O, typename... Args> QueueSyncContext(Q& q, F&& work, O&& obj, Args&&... args):
        m_queue{q},
        m_thread{std::thread{work, std::forward<O>(obj), std::ref(m_mutex), std::ref(m_cv),
                 std::ref(m_ready), std::forward<Args>(args)...}}
    {
        m_thread.detach();
    }

    ~QueueSyncContext()
    {
        wait();
    }

    void wait()
    {
//        while(!m_queue.empty())
//        {
//            using namespace std::chrono_literals;
//            std::this_thread::sleep_for(1ms);
//        }
        while(!m_ready)
        {
            std::unique_lock lk{m_mutex};
            while(!m_ready)
                m_cv.wait(lk);
        }
    }

//    void push

//private:
    Q& m_queue;
    std::atomic<bool> m_ready{true};
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_thread;
};


class LogQueue
{
public:
    void wait();

    void push(CmdCollector::cmds_t& cmds);

private:
    void work(std::mutex& m, std::condition_variable& cv, std::atomic<bool>& ready);

    using queue_t = std::deque<CmdCollector::cmds_t>;

    queue_t m_to_log_q;
    std::mutex m_logq_mutex;
    QueueSyncContext<queue_t> m_log{m_to_log_q, &LogQueue::work, this};
};

class FilesQueue
{
public:
    void wait();

    void push(std::pair<std::string, CmdCollector::cmds_t>&& element);

private:
    void work(std::mutex& m, std::condition_variable& cv, std::atomic<bool>& ready);

    using queue_t = std::deque<std::pair<std::string, CmdCollector::cmds_t>>;

    queue_t m_to_files_q;

    std::mutex m_filesq_mutex;

    QueueSyncContext<queue_t> m_f1{m_to_files_q, &FilesQueue::work, this};
    QueueSyncContext<queue_t> m_f2{m_to_files_q, &FilesQueue::work, this};
};


}

