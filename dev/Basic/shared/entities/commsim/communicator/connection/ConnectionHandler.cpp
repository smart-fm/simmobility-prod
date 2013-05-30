/*
 * ConnectionHandler.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "ConnectionHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "Session.hpp"

namespace sim_mob {
ConnectionHandler::ConnectionHandler(
		session_ptr session_ ,
		Broker& broker,
		messageReceiveCallback callback,
		unsigned int clientID_,
		unsigned int ClienType_ ,
		unsigned long int agentPtr_
		):theBroker(broker), receiveCallBack(callback)

{
	mySession = session_;
	clientID = clientID_;
	clientType = ClienType_;
	agentPtr = agentPtr_;
}

void ConnectionHandler::start()
{

	Json::Value ready;
	ready["MessageType"] = "Ready";
	Json::FastWriter writer;

	std::string str = writer.write(ready);

	mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
}

void ConnectionHandler::readyHandler(const boost::system::error_code &e, std::string str)
{
	if(e)
	{
		std::cerr << "Connection Not Ready [" << e.message() << "] Trying Again" << std::endl;
		mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
	}
	else
	{
		//will not pass 'message' variable as argument coz it
		//is global between functions. some function read into it, another function read from it
		mySession->async_read(incomingMessage,
			boost::bind(&ConnectionHandler::readHandler, this,
					boost::asio::placeholders::error));
	}
}

void ConnectionHandler::readHandler(const boost::system::error_code& e) {

	if(e)
	{
		std::cerr << "Read Fail [" << e.message() << "]" << std::endl;
	}
	else
	{
		//call the receive handler in the broker
		CALL_MEMBER_FN(theBroker, receiveCallBack)(shared_from_this(),incomingMessage);

		mySession->async_read(incomingMessage,
						boost::bind(&ConnectionHandler::readHandler, this,
								boost::asio::placeholders::error));
	}
}

void ConnectionHandler::send(std::string str)
{
	mySession->async_write(str,boost::bind(&ConnectionHandler::sendHandler, this, boost::asio::placeholders::error));
}
void ConnectionHandler::sendHandler(const boost::system::error_code& e) {
	std::cout << "Write to agent[" << agentPtr << "]  client["  << clientID << "] " <<(e?"Failed":"Success") << std::endl;
}
} /* namespace sim_mob */
