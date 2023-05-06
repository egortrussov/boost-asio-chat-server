//
// Created by yegor on 06.05.23.
//

#include "Session.h"

#include <iostream>

Session::Session(tcp::socket socket, Room &room) : room_(room), socket_(std::move(socket)) {
}

void Session::Start() {
    room_.Join(shared_from_this());
    ReadHeader();
}

void Session::Deliver(const Message &message) {
    bool write_in_progress = !message_queue_.empty();
    message_queue_.push_back(message);
    if (!write_in_progress) {
        Write();
    }
}

void Session::ReadHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.Data(), Message::HEADER_LENGTH),
                            [this, self](boost::system::error_code ec, size_t) {
                                if (!ec && message_.DecodeHeader()) {
                                    ReadBody();
                                } else {
                                    room_.Leave(shared_from_this());
                                }
                            });
}

void Session::ReadBody() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(message_.Body(), message_.BodyLength()),
                            [this, self](boost::system::error_code ec, size_t) {
                                if (!ec) {
                                    room_.Deliver(message_);
                                    ReadHeader();
                                } else {
                                    room_.Leave(shared_from_this());
                                }
                            });
}

void Session::Write() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
                             boost::asio::buffer(message_queue_.front().Data(), message_queue_.front().Length()),
                             [this, self](boost::system::error_code ec, size_t) {
                                 if (!ec) {
                                     message_queue_.pop_front();
                                     if (!message_queue_.empty()) {
                                         Write();
                                     }
                                 } else {
                                     room_.Leave(shared_from_this());
                                 }
                             });
}

