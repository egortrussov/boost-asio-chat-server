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
};


#endif //BOOST_CHAT_USER_H
