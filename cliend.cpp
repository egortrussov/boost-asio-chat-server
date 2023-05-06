//
// Created by yegor on 06.05.23.
//

#include <iostream>
#include <thread>
#include <cstdlib>
#include <deque>
#include <boost/asio.hpp>
#include "Message.h"
using boost::asio::ip::tcp;
using namespace std;
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
