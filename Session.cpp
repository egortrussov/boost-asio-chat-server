//
// Created by yegor on 06.05.23.
//

#include "Session.h"

#include <iostream>

Session::Session(tcp::socket socket, Room &room, std::unordered_map<std::string, Room>& rooms_data) : room_(room), socket_(std::move(socket)), rooms_data_(rooms_data) {
}

void Session::Start() {
    rooms_data_[current_room_id_].Join(shared_from_this());
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
                                    rooms_data_[current_room_id_].Leave(shared_from_this());
                                }
                            });
}

void Session::ReadBody() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_, boost::asio::buffer(message_.Body(), message_.BodyLength()),
                            [this, self](boost::system::error_code ec, size_t) {
                                if (!ec) {
                                    std::string msg = message_.GetBodyString();
                                    if (msg[0] == '&') {
                                        const std::string msg_code = msg.substr(1, msg.size() - 1);
                                        rooms_data_[current_room_id_].Leave(shared_from_this());
                                        rooms_data_[msg_code].Join(shared_from_this());
                                        current_room_id_ = msg_code;
                                    } else {
                                        rooms_data_[current_room_id_].Deliver(message_);
                                    }
                                    ReadHeader();
                                } else {
                                    rooms_data_[current_room_id_].Leave(shared_from_this());
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
                                     rooms_data_[current_room_id_].Leave(shared_from_this());
                                 }
                             });
}

