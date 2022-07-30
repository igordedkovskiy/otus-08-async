// homework #8: async command processing (bulk).

#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include "async.h"

void single_thread()
{
    constexpr async::size_type bulk_size{3};

    const auto h1{async::connect(bulk_size)};
    async::receive(h1, "cmd1\ncmd2\ncmd3\ncmd4\ncmd5\n", 25);
    async::disconnect(h1);

    {
        std::cout << std::endl;
        const auto h1{async::connect(bulk_size)};
        const auto h2{async::connect(bulk_size)};
        async::receive(h2, "cmd1\n", 5);
        async::receive(h1, "cmd1\ncmd2\n", 10);
        async::receive(h2, "cmd2\n{\ncmd3\ncmd4\n}\n", 19);
        async::receive(h2, "{\n", 2);
        async::receive(h2, "cmd5\ncmd6\n{\ncmd7\ncmd8\n}\ncmd9\n}\n", 31);
        async::receive(h1, "cmd3\ncmd4\n", 10);
        async::receive(h2, "{\ncmd10\ncmd11\n", 12);
        async::receive(h1, "cmd5\n", 5);
        async::disconnect(h2);
        async::disconnect(h1);
    }
    {
        std::cout << std::endl;
        const auto h1{async::connect(bulk_size)};
        async::receive(h1, "cmd1\n", 5);
        async::receive(h1, "cmd2\n", 5);
        async::receive(h1, "cmd3\n", 5);
        async::receive(h1, "cmd4\n", 5);
        async::receive(h1, "cmd5\n", 5);
        async::disconnect(h1);
    }
//    async::wait();
}

int main()
{
    std::cout << "single thread:\n";
    single_thread();
    return 0;
}
