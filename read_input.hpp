#pragma once

#include <iostream>
#include <utility>
#include <limits>
#include <exception>
#include <limits>

std::size_t read_input_size(std::istream& in_stream, std::ostream& err_stream);

template<typename F, typename ParseErr>
void read_input(std::istream& in_stream, std::ostream& err_stream, F process)
{
    std::string read;
    std::size_t lines_cntr = 0;
    std::size_t last_failed_line = std::numeric_limits<decltype(last_failed_line)>::max();
    while(in_stream >> read)
    {
        ++lines_cntr;
        {
            try
            {
                process(std::move(read));
                read.clear();
            }
            catch(ParseErr)
            {
                err_stream << "Failed to process input at line "
                           << lines_cntr
                           << ": incorrect format\n";
                last_failed_line = lines_cntr;
                --lines_cntr;
            }
            catch(std::exception& e)
            {
                err_stream << "Failed to process input at line "
                           << lines_cntr << ": "
                           << e.what()
                           << '\n';
                break;
            }
        }
        in_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}
