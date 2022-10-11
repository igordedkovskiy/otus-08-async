#include <algorithm>
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <limits>

#include "read_input.hpp"

std::size_t read_input_size(std::istream &in_stream, std::ostream& err_stream)
{
    std::string in_size_s;
    long in_size;
    while(true)
    {
        in_stream >> in_size_s;
        try
        {
            in_size = stoi(in_size_s);
            in_size_s.clear();
            if(in_size < 0)
                continue;
            break;
        }
        catch(std::logic_error& e)
        {
            err_stream << "Failed to process input size: incorrect format" << std::endl;
        }
    }
    return in_size;
}
