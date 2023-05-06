//
// Created by yegor on 06.05.23.
//

#include "Server.h"

#include <iostream>

Server::Server(boost::asio::io_context &io_context, const int32_t port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    DoAccept();
}

void Server::DoAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<Session>(std::move(socket), room_)->Start();
        }
        DoAccept();
    });
}
