//
// Created by yegor on 06.05.23.
//

#ifndef BOOST_CHAT_SERVER_H
#define BOOST_CHAT_SERVER_H

#include "Room.h"
#include "Session.h"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, const int32_t port);

private:
    void DoAccept();

    tcp::acceptor acceptor_;
    Room room_;
};


#endif //BOOST_CHAT_SERVER_H
