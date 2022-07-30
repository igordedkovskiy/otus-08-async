#pragma once

#include <cstdint>

extern "C"
{

namespace async
{

using size_type = std::size_t;
using handler_t = void*;

/// \brief   Create a new context
/// \arg \a  bulk_size is a size of a block of commands
/// \returns a handler to context
handler_t connect(size_type bulk_size);

int disconnect(handler_t h);

/// \brief Transfer commands to a context
void receive(handler_t h, const char* data, size_type data_size);

/// \brief Block current thread until library has finished processing a data
/// \note  If you need access to an output files, then you need to wait
///        until files are closed by a library.
void wait();

}

}
