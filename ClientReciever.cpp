//
// Created by yegor on 08.05.23.
//

#include "ClientReciever.h"

#include <iostream>

ClientReciever::ClientReciever(boost::asio::io_context &context, const tcp::resolver::results_type &endpoints)
        : context_(context), socket_(context) {
    Connect(endpoints);
}

void ClientReciever::Write(const Message &message) {
    boost::asio::post(context_, [this, &message]() {
        bool write_in_progress = !message_queue_.empty();
        message_queue_.push_back(message);
        if (!write_in_progress) {
            WriteResponse();
        }
    });
}

void ClientReciever::Close() {
    boost::asio::post(context_, [this]() { socket_.close(); });
}

void ClientReciever::Connect(const tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, tcp::endpoint) {
        if (!ec) {
            ReadHeader();
        }
    });
}

void ClientReciever::ReadHeader() {
    boost::asio::async_read(socket_, boost::asio::buffer(read_message_.Data(), Message::HEADER_LENGTH),
                            [this](boost::system::error_code ec, size_t) {
                                if (!ec && read_message_.DecodeHeader()) {
                                    ReadBody();
                                } else {
                                    socket_.close();
                                }
                            });
}

void ClientReciever::ReadBody() {
    boost::asio::async_read(socket_, boost::asio::buffer(read_message_.Body(), read_message_.BodyLength()),
                            [this](boost::system::error_code ec, size_t) {
                                if (!ec) {
                                    std::cout.write(read_message_.Body(), read_message_.BodyLength());
                                    std::cout << "\n";
                                    ReadHeader();
                                } else {
                                    socket_.close();
                                }
                            });
}

void ClientReciever::WriteResponse() {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(message_queue_.front().Data(), message_queue_.front().Length()),
                             [this](boost::system::error_code ec, size_t) {
                                 if (!ec) {
                                     message_queue_.pop_front();
                                     if (!message_queue_.empty()) {
                                         WriteResponse();
                                     }
                                 } else {
                                     socket_.close();
                                 }
                             });
}


