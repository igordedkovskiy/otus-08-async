#include <sstream>
#include <fstream>
#include "CmdCollector.hpp"

CmdCollector::CmdCollector(std::size_t N):
    m_block_size{N}
{
    m_cmds.reserve(N);
}

bool CmdCollector::input_block_finished() const noexcept
{
    return m_block_finished;
}

void CmdCollector::finish_block() noexcept
{
    if(m_type == BlockType::STATIC)
        m_block_finished = true;
}

std::size_t CmdCollector::block_size() const noexcept
{
    return m_cmds.size();
}

bool CmdCollector::remaining_data_valid() const noexcept
{
    if(m_type == BlockType::STATIC)
        return block_size();
    return block_size() && m_block_finished;
}

void CmdCollector::process_cmd(std::string &&cmd)
{
    if(cmd == "{")
    {
        m_type = BlockType::DYNAMIC;
        if(++m_braces == 1)
        {
            if(m_cmds.size() > 0)
                m_block_finished = true;
        }
    }
    else if(cmd == "}")
    {
        if(m_braces == 0)
            throw ParseErr::incorrect_format;
        if(--m_braces == 0)
        {
            m_type = BlockType::STATIC;
            m_block_finished = true;
        }
    }
    else
    {
        m_cmds.emplace_back(std::move(cmd));
        if(m_cmds.size() == 1)
            m_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if(m_type == BlockType::STATIC)
        {
            if(m_cmds.size() == m_block_size)
                m_block_finished = true;
        }
    }
}

time_t CmdCollector::block_start_time([[maybe_unused]] std::size_t number) const noexcept
{
    return m_time;
}

void CmdCollector::set_block_max_size(std::size_t N)
{
    m_block_size = N;
    m_cmds.reserve(N);
}

void CmdCollector::clear_commands() noexcept
{
    m_time = 0;
    m_cmds.clear();
    m_block_finished = false;
}

void CmdCollector::reset() noexcept
{
    m_type = BlockType::STATIC;
    m_braces = 0;
    clear_commands();
}

generator<std::string> CmdCollector::get_cmd() const
{
    for(const auto& cmd:m_cmds)
        co_yield cmd;
}

CmdCollector::cmds_t &CmdCollector::get_cmds() noexcept
{
    return m_cmds;
}

