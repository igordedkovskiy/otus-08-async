#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include "gtest/gtest.h"

#include "CmdCollector.hpp"
#include "read_input.hpp"
#include "async.hpp"

TEST(TEST_ASYNC, task_example)
{
    CmdCollector commands{3};

    auto print = [&commands](std::stringstream& out)
    {
        out << "bulk: ";
        std::size_t cntr = 0;
        for(const auto& cmd:commands.get_cmd())
        {
            out << cmd;
            if(++cntr < commands.block_size())
                out << ", ";
        }
        out << '\n';
        commands.clear_commands();
    };

    std::stringstream result;
    auto process = [&commands, &print, &result](std::string&& read)
    {
        commands.process_cmd(std::move(read));
        read.clear();
        if(commands.input_block_finished())
            print(result);
    };

    {
        std::stringstream input{"cmd1\n"
                                "cmd2\n"
                                "{\n"
                                "cmd3\n"
                                "cmd4\n"
                                "}\n"
                                "{\n"
                                "cmd5\n"
                                "cmd6\n"
                                "{\n"
                                "cmd7\n"
                                "cmd8\n"
                                "}\n"
                                "cmd9\n"
                                "}\n"
                                "{\n"
                                "cmd10\n"
                                "cmd11"};

        read_input<decltype(process), CmdCollector::ParseErr>(input, std::cerr, process);
        commands.finish_block();
        if(commands.input_block_finished())
            print(result);

        std::stringstream ref{"bulk: cmd1, cmd2\n"
                              "bulk: cmd3, cmd4\n"
                              "bulk: cmd5, cmd6, cmd7, cmd8, cmd9\n"};
        ASSERT_TRUE(result.str() == ref.str());
    }

    result.str("");
    commands.reset();
    {
        std::stringstream input{"cmd1\n"
                                "cmd2\n"
                                "cmd3\n"
                                "cmd4\n"
                                "cmd5"};

        read_input<decltype(process), CmdCollector::ParseErr>(input, std::cerr, process);
        commands.finish_block();
        if(commands.input_block_finished())
            print(result);

        std::stringstream ref{"bulk: cmd1, cmd2, cmd3\n"
                              "bulk: cmd4, cmd5\n"};
        ASSERT_TRUE(result.str() == ref.str());
    }
}

TEST(TEST_ASYNC, test_lib)
{
    constexpr size_type bulk_size{3};
    const auto h1{connect(bulk_size)};
    const auto h2{connect(bulk_size)};

    const char* commands1[] = {"cmd1", "cmd2", "cmd3", "cmd4", "cmd5"};
    constexpr size_type num1 = sizeof(commands1) / sizeof(commands1[0]);

    const char* commands2[] = {"cmd1", "cmd2",
                               "{", "cmd3", "cmd4", "}",
                               "{", "cmd5", "cmd6", "{", "cmd7", "cmd8", "}", "cmd9", "}",
                               "{", "cmd10", "cmd11"
                              };
    constexpr size_type num2 = sizeof(commands2) / sizeof(commands2[0]);

    receive(h2, commands2, num2);
    disconnect(h2);

    receive(h1, commands1, num1);
    disconnect(h1);

    auto find_file = [](std::string masks)
    {
        const auto regexp_cmp{std::regex(masks)};
        namespace fs = std::filesystem;
        const auto path{fs::absolute(".")};
        using iterator = fs::directory_iterator;
        std::vector<std::string> fnames;
        for(auto it{iterator(path)}; it != iterator(); ++it)
        {
            const auto path{fs::absolute(it->path())};
            if(!fs::is_directory(path))
            {
                const auto fname{path.filename().string()};
                if(regex_match(fname, regexp_cmp))
                    fnames.emplace_back(std::move(fname));
            }
        }
        return fnames;
    };

    auto check = [&find_file](std::stringstream ref, std::string masks)
    {
        const auto fname{find_file(std::move(masks))};
        if(fname.empty())
            return false;
        for(const auto& fn:fname)
        {
            std::fstream file{fn, std::fstream::in};
            std::stringstream out;
            out << file.rdbuf();
            if(out.str() != ref.str())
                return false;
        }
        return true;
    };

    // context 1
    ASSERT_TRUE(check(std::stringstream{"bulk: cmd1, cmd2, cmd3\n"}, std::string{"(1-bulk.*-1.log)"}));
    ASSERT_TRUE(check(std::stringstream{"bulk: cmd4, cmd5\n"}, std::string{"(1-bulk.*-2.log)"}));

    // context 2
    ASSERT_TRUE(check(std::stringstream{"bulk: cmd1, cmd2\n"}, std::string{"(2-bulk.*-1.log)"}));
    ASSERT_TRUE(check(std::stringstream{"bulk: cmd3, cmd4\n"}, std::string{"(2-bulk.*-2.log)"}));
    ASSERT_TRUE(check(std::stringstream{"bulk: cmd5, cmd6, cmd7, cmd8, cmd9\n"}, std::string{"(2-bulk.*-3.log)"}));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
