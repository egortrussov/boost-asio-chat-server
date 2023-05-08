//
// Created by yegor on 06.05.23.
//

#ifndef BOOST_CHAT_USER_H
#define BOOST_CHAT_USER_H

#include "Message.h"

class User {
public:
    virtual void Deliver(const Message& message) = 0;
    virtual ~User() = default;

protected:
    std::string current_room_id_;
    std::string username_;
};


#endif //BOOST_CHAT_USER_H
