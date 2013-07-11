/*
 * ConnectionHandler.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "ConnectionHandler.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"
#include "Session.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"

namespace sim_mob {
ConnectionHandler::ConnectionHandler(
		session_ptr session_ ,
		Broker& broker,
		messageReceiveCallback callback,
		std::string clientID_,
		unsigned int ClienType_ ,
		unsigned int agentPtr_
		):theBroker(broker), receiveCallBack(callback)

{
	mySession = session_;
	clientID = clientID_;
	clientType = ClienType_;
	agentPtr = agentPtr_;
//	incomingMessage = "'\0'";
	valid = true;
}

ConnectionHandler::~ConnectionHandler(){
	mySession.reset();
}
void ConnectionHandler::start()
{


	Json::Value packet;
	Json::Value packet_header = JsonParser::createPacketHeader(pckt_header(1));
	Json::Value msg = JsonParser::createMessageHeader(msg_header("0","SIMMOBILITY","READY"));
	packet["PACKET_HEADER"] = packet_header;
	packet["DATA"].append(msg);//no other data element needed
	Json::FastWriter writer;

	std::string readyMessage = writer.write(packet);
	mySession->async_write(readyMessage,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,readyMessage));
}

void ConnectionHandler::readyHandler(const boost::system::error_code &e, std::string str)
{
	if(e)
	{
		std::cerr << "Connection Not Ready[" << e.message() << "] Trying Again" << std::endl;
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
//	Print() << "Write to agent[" << agentPtr << "]  client["  << clientID << "] " <<(e?"Failed":"Success") << std::endl;
}

bool ConnectionHandler::is_open(){
	return mySession->socket().is_open();
}


bool ConnectionHandler::isValid() {
	return valid;
}

void ConnectionHandler::setValidation(bool value) {
	valid = value;
}
} /* namespace sim_mob */
