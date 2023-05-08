//
// Created by yegor on 06.05.23.
//

#ifndef BOOST_CHAT_MESSAGE_H
#define BOOST_CHAT_MESSAGE_H


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

class Message {
public:
    static const size_t HEADER_LENGTH = 4;
    static const size_t MAX_BODY_LENGTH = 1024;

    Message();

    const char* Data() const;
    char* Data();

    size_t Length();

    const char* Body() const;
    char* Body();

    size_t BodyLength() const;
    void BodyLength(const size_t new_size);

    bool DecodeHeader();
    void EncodeHeader();

    std::string GetBodyString() const;
    std::string GetMessageString() const;


    void SetBody(const std::string& msg);
private:
    char data_[HEADER_LENGTH + MAX_BODY_LENGTH];
    size_t body_size_;
};


#endif //BOOST_CHAT_MESSAGE_H
