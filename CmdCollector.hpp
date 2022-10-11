#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "generator.hpp"

class CmdCollector
{
public:
    enum class ParseErr: std::uint8_t
    {
        incorrect_format
    };

    CmdCollector() noexcept = default;
    CmdCollector(const CmdCollector&) = default;
    CmdCollector(CmdCollector&&) noexcept = default;
    CmdCollector& operator=(const CmdCollector&) = default;
    CmdCollector& operator=(CmdCollector&&) noexcept = default;
    ~CmdCollector() noexcept = default;

    CmdCollector(std::size_t N);

    bool input_block_finished() const noexcept;
    void finish_block() noexcept;

    std::size_t block_size() const noexcept;

    bool remaining_data_valid() const noexcept;

    void process_cmd(std::string&& cmd);

    time_t block_start_time([[maybe_unused]] std::size_t number) const noexcept;

    void set_block_max_size(std::size_t N);

    void clear_commands() noexcept;

    void reset() noexcept;

    generator<std::string> get_cmd() const;

    using cmds_t = std::vector<std::string>;
    cmds_t& get_cmds() noexcept;

private:
    enum class BlockType { STATIC, DYNAMIC };

    BlockType m_type{BlockType::STATIC};
    bool m_block_finished{false};
    std::size_t m_block_size{0};
    std::size_t m_braces{0};
    std::time_t m_time;
    cmds_t m_cmds;
};
