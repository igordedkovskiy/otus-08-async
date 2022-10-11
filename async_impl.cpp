#include <iostream>
#include <sstream>
#include <fstream>

#include "async_impl.hpp"

namespace async
{

Handlers::handlers_t::iterator Handlers::find(handler_t h)
{
    return m_handlers.find(h);
}

Handlers::handlers_t::iterator Handlers::end()
{
    return m_handlers.end();
}

handler_t Handlers::create(std::unique_ptr<CmdCollector> collector)
{
    const auto ptr{collector.get()};
    m_handlers.insert(std::make_pair(ptr, std::move(collector)));
    return ptr;
}

void Handlers::destroy(handler_t h)
{
    auto el{m_handlers.find(h)};
    if(el != std::end(m_handlers))
        m_handlers.erase(el);
}

size_type Handlers::size() const noexcept
{
    return m_handlers.size();
}

void LogQueue::work(std::mutex& m, std::condition_variable& cv, std::atomic<bool>& ready)
{
    auto get = [this](CmdCollector::cmds_t& cmds)
    {
        {
            std::scoped_lock lk{m_logq_mutex};
            if(m_to_log_q.empty())
                return false;
            cmds = std::move(m_to_log_q.front());
            m_to_log_q.pop_front();
        }
        return true;
    };

    auto& stream{std::cout};
    CmdCollector::cmds_t cmds;
    while(true)
    {
        {
            std::scoped_lock lk{m};
            while(get(cmds))
            {
                stream << "bulk: ";
                std::size_t cntr = 0;
                for(const auto& cmd:cmds)
                {
                    stream << cmd;
                    if(++cntr < cmds.size())
                        stream << ", ";
                }
                stream << '\n';
            }
            ready = true;
        }
        cv.notify_all();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }
}

void FilesQueue::work(std::mutex& m, std::condition_variable& cv, std::atomic<bool> &ready)
{
    auto get = [this](std::string& fname, CmdCollector::cmds_t& cmds)
    {
        {
            std::scoped_lock lk{m_filesq_mutex};
            if(m_to_files_q.empty())
                return false;
            fname = std::move(m_to_files_q.front().first);
            cmds = std::move(m_to_files_q.front().second);
            m_to_files_q.pop_front();
        }
        return true;
    };

    std::string fname;
    CmdCollector::cmds_t cmds;
    while(true)
    {
        {
            std::scoped_lock lk{m};
            while(get(fname, cmds))
            {
                auto stream{std::fstream{fname, std::fstream::out | std::fstream::app}};
                stream << "bulk: ";
                std::size_t cntr = 0;
                for(const auto& cmd:cmds)
                {
                    stream << cmd;
                    if(++cntr < cmds.size())
                        stream << ", ";
                }
                stream << '\n';
            }
            ready = true;
        }
        cv.notify_all();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }
}

void LogQueue::wait()
{
//    auto empty = [](auto& q, auto& mutex)
//    {
//        std::scoped_lock lk{mutex};
//        return q.empty();
//    };

//    while(!empty(m_to_log_q, m_logq_mutex) || !empty(m_to_files_q, m_filesq_mutex))
//    {
//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(1ms);
//    }

    m_log.wait();
}

void FilesQueue::wait()
{
    m_f2.wait();
    m_f1.wait();
}

void LogQueue::push(CmdCollector::cmds_t& cmds)
{
    std::lock_guard lk{m_logq_mutex};
    m_to_log_q.push_back(cmds);
    m_log.m_ready = false;
}

void FilesQueue::push(std::pair<std::string, CmdCollector::cmds_t>&& element)
{
    std::lock_guard lk{m_filesq_mutex};
    m_to_files_q.emplace_back(std::forward<decltype(element)>(element));
    m_f1.m_ready = false;
    m_f2.m_ready = false;
}

}
