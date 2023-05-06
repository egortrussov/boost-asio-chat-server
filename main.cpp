#include <iostream>
#include <boost/asio.hpp>

#include "Server.h"
#include <list>

int main(int argc, char* argv[]) {
    try {
        if(argc < 2) {
            std::cerr << "Usage: server <port> [<port> ...]\n";
            return 1;
        }
        boost::asio::io_context io_context;
        std::list<Server> servers;
        for(int i = 1; i < argc; ++i) {
//            tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
            servers.emplace_back(io_context, std::atoi(argv[i]));
        }
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
//    try {
//        boost::asio::io_context io_context;
//        Server server(io_context, 8080);
//        io_context.run();
//    } catch (const std::exception& e) {
//        std::cout << "Crashed with err: " + std::string(e.what());
//    }
//    return 0;
}
