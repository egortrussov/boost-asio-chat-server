//
// Created by yegor on 06.05.23.
//

#ifndef BOOST_CHAT_SESSION_H
#define BOOST_CHAT_SESSION_H

#include "User.h"
#include "Room.h"

#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session : public User, public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::unordered_map<std::string, Room>& rooms_data_);

    void Start();
    void Deliver(const Message& message) override;

    ~Session() override = default;

private:

    void ReadHeader();
    void ReadBody();
    void Write();

    bool HandleMessageCommands(const std::string& msg);
    void FormatMessageBeforeDelivery();

    std::deque<Message> message_queue_;
    tcp::socket socket_;
    Message message_;
    std::unordered_map<std::string, Room>& rooms_data_;
};


#endif //BOOST_CHAT_SESSION_H
