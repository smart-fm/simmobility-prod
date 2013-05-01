#pragma once
#include "message/Message.hpp"
#include "../Server/ASIO_Server.hpp"

namespace sim_mob {
class ConnectionHandler;
    /**
     * Represents a message data that can be exchanged on Messaging Entities.
     */
    class BrokerMessage : public Message
    {
    public:
    	Json::Value root;//let's see if it comes handy
    	std::string str;
    	sim_mob::ConnectionHandler* temp;
    public:
//        DataMessage();
    	BrokerMessage(std::string str_, Json::Value root, ConnectionHandler* temp):str(str_), root(root), temp(temp){}
//        DataMessage(const Message& orig);
        virtual ~BrokerMessage(){};
    };
}
