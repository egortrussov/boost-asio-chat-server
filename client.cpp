//
// Created by yegor on 08.05.23.
//

#include <boost/asio.hpp>
#include <iostream>
#include <thread>

#include "Message.h"
#include "ClientReciever.h"

using boost::asio::ip::tcp;

namespace {
    Message CreateMessage(const char line[]) {
        Message msg;
        msg.BodyLength(strlen(line));
        memcpy(msg.Body(), line, msg.BodyLength());
        msg.EncodeHeader();
        return msg;
    }
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }
        boost::asio::io_context context;
        tcp::resolver resolver(context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        ClientReciever c(context, endpoints);
        std::thread t([&context]() { context.run(); });
        char line[Message::MAX_BODY_LENGTH + 1];
        while (std::cin.getline(line, Message::MAX_BODY_LENGTH + 1)) {
            Message messageItem = CreateMessage(line);
            c.Write(messageItem);
        }
        c.Close();
        t.join();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
