//
// Created by yegor on 06.05.23.
//

#include <iostream>
#include <thread>
#include <cstdlib>
#include <deque>
#include <boost/asio.hpp>
//#include "Message.h"
using boost::asio::ip::tcp;
using namespace std;

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

std::string Message::GetMessageString() const {
    std::string str;
    for (size_t index = 0; index < HEADER_LENGTH + body_size_; ++index) {
        str += data_[index];
    }
    return str;
}

void Message::SetBody(const std::string &msg) {
    if (msg.size() > MAX_BODY_LENGTH) {
        return;
    }
    body_size_ = msg.size();
    for (size_t index = 0; index < msg.size(); ++index) {
        data_[HEADER_LENGTH + index] = msg[index];
    }
    EncodeHeader();
}



typedef deque<Message> messageQueue;
class client {
public:
    client(boost::asio::io_context& context, const tcp::resolver::results_type& endpoints) : context(context), socket(context) {
        connect(endpoints);
    }
    void write(const Message& messageItem) {
        boost::asio::post(context, [this, messageItem]() {
            bool write_in_progress = !writeMessage.empty();
            writeMessage.push_back(messageItem);
            if(!write_in_progress) {
                write();
            }
        });
    }
    void close() {
        boost::asio::post(context, [this]() { socket.close(); });
    }
private:
    void connect(const tcp::resolver::results_type& endpoints) {
        boost::asio::async_connect(socket, endpoints, [this](boost::system::error_code ec, tcp::endpoint) {
            if(!ec) {
                readHeader();
            }
        });
    }
    void readHeader() {
        boost::asio::async_read(socket, boost::asio::buffer(readMessage.Data(), Message::HEADER_LENGTH), [this](boost::system::error_code ec, size_t) {
            if(!ec && readMessage.DecodeHeader()) {
                readBody();
            }
            else {
                socket.close();
            }
        });
    }
    void readBody() {
        boost::asio::async_read(socket, boost::asio::buffer(readMessage.Body(), readMessage.BodyLength()), [this](boost::system::error_code ec, size_t) {
            if(!ec) {
                cout.write(readMessage.Body(), readMessage.BodyLength());
                cout << "\n";
                readHeader();
            }
            else {
                socket.close();
            }
        });
    }
    void write() {
        boost::asio::async_write(socket, boost::asio::buffer(writeMessage.front().Data(), writeMessage.front().Length()), [this](boost::system::error_code ec, size_t) {
            if(!ec) {
                writeMessage.pop_front();
                if(!writeMessage.empty()) {
                    write();
                }
            }
            else {
                socket.close();
            }
        });
    }
    boost::asio::io_context& context;
    tcp::socket socket;
    Message readMessage;
    std::deque<Message> writeMessage;
};
int main(int argc, char* argv[]) {
    try {
        if(argc != 3) {
            cerr << "Usage: client <host> <port>\n";
            return 1;
        }
        boost::asio::io_context context;
        tcp::resolver resolver(context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);
        client c(context, endpoints);
        thread t([&context](){ context.run(); });
        char line[Message::MAX_BODY_LENGTH + 1];
        while(cin.getline(line, Message::MAX_BODY_LENGTH + 1)) {
            Message messageItem;
            messageItem.BodyLength(strlen(line));
            memcpy(messageItem.Body(), line, messageItem.BodyLength());
            messageItem.EncodeHeader();
            c.write(messageItem);
        }
        c.close();
        t.join();
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
