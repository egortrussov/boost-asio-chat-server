//
// Created by yegor on 06.05.23.
//

#ifndef BOOST_CHAT_ROOM_H
#define BOOST_CHAT_ROOM_H

#include "User.h"
#include "Message.h"

#include <memory>
#include <deque>
#include <set>

class Room {
private:
    typedef std::shared_ptr<User> UserPointer;

public:
    void Join(UserPointer user);
    void Deliver(const Message& message);
    void Leave(UserPointer user);

private:
    std::deque<Message> message_queue_;
    std::set<UserPointer> users_;
};


#endif //BOOST_CHAT_ROOM_H
