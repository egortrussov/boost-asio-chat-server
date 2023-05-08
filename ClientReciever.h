//
// Created by yegor on 08.05.23.
//

#ifndef BOOST_CHAT_CLIENTRECIEVER_H
#define BOOST_CHAT_CLIENTRECIEVER_H

#include "Message.h"

#include <boost/asio.hpp>
#include <deque>

using boost::asio::ip::tcp;

class ClientReciever {
public:
    ClientReciever(boost::asio::io_context &context, const tcp::resolver::results_type &endpoints);

    void Write(const Message &message_item);

    void Close();

private:
    typedef std::deque<Message> MessageQueue;

    void Connect(const tcp::resolver::results_type &endpoints);

    void ReadHeader();

    void ReadBody();

    void WriteResponse();

    boost::asio::io_context &context_;
    tcp::socket socket_;
    Message read_message_;
    MessageQueue message_queue_;
};


#endif //BOOST_CHAT_CLIENTRECIEVER_H
