//
// Created by yegor on 06.05.23.
//

#include "Session.h"

#include <iostream>

namespace {
    std::vector<std::pair<std::string, std::string>> ParseMessageCommands(const std::string& msg) {
        static const char16_t COMMAND_SYMBOL = '/';
        static const char16_t COMMAND_VALUE_SYMBOL = ':';
        if (!std::count(msg.begin(), msg.end(), COMMAND_SYMBOL)) {
            return {};
        }
        std::vector<std::pair<std::string, std::string>> result;
        size_t current_pos = msg.find(COMMAND_SYMBOL);
        while (current_pos < msg.size()) {
            size_t value_begin = msg.find(COMMAND_VALUE_SYMBOL, current_pos);
            size_t value_end = msg.find(COMMAND_SYMBOL, current_pos + 1);
            if (value_end == std::string::npos) {
                value_end = msg.size();
            }
            result.emplace_back(msg.substr(current_pos + 1, value_begin - current_pos - 1),
                                msg.substr(value_begin + 1, value_end - value_begin - 1));
            current_pos = value_end;
        }
        return result;
    }
}

Session::Session(tcp::socket socket, std::unordered_map<std::string, Room>& rooms_data) : socket_(std::move(socket)), rooms_data_(rooms_data) {
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
                                    if (!HandleMessageCommands(msg)) {
                                        FormatMessageBeforeDelivery();
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

bool Session::HandleMessageCommands(const std::string& msg) {
    const std::vector<std::pair<std::string, std::string>> commands = ParseMessageCommands(msg);
    if (commands.empty()) {
        return false;
    }

    for (const auto& [command, value] : commands) {
        if (command == "swr") {
            rooms_data_[current_room_id_].Leave(shared_from_this());
            rooms_data_[value].Join(shared_from_this());
            current_room_id_ = value;
        }
        if (command == "usr") {
            username_ = value;
        }
    }
    return true;
}

void Session::FormatMessageBeforeDelivery() {
    auto tm = message_.GetBodyString();
    message_.SetBody(username_ + ": " + message_.GetBodyString());
    std::cout << message_.GetBodyString() << std::endl;
}