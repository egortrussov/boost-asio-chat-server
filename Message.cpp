//
// Created by yegor on 06.05.23.
//

#include "Message.h"

Message::Message() : body_size_(0) {
}

char *Message::Data() {
    return data_;
}

const char *Message::Data() const {
    return data_;
}

size_t Message::Length() {
    return HEADER_LENGTH + body_size_;
}

const char *Message::Body() const {
    return data_ + HEADER_LENGTH;
}

char *Message::Body() {
    return data_ + HEADER_LENGTH;
}

size_t Message::BodyLength() const {
    return body_size_;
}

void Message::BodyLength(const size_t new_size) {
    body_size_ = std::min(size_t(1024), new_size);
}

bool Message::DecodeHeader() {
    char header[HEADER_LENGTH + 1] = "";
    strncat(header, data_, HEADER_LENGTH);
    body_size_ = atoi(header);
    if (body_size_ > MAX_BODY_LENGTH) {
        body_size_ = 0;
        return false;
    }
    return true;
}

void Message::EncodeHeader() {
    char header[HEADER_LENGTH + 1] = "";
    sprintf(header, "%4d", static_cast<int>(body_size_));
    memcpy(data_, header, HEADER_LENGTH);
}

std::string Message::GetBodyString() const {
    std::string str;
    for (size_t index = HEADER_LENGTH; index < HEADER_LENGTH + body_size_; ++index) {
        str += data_[index];
    }
    return str;
}