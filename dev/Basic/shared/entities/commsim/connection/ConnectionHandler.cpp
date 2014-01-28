//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ConnectionHandler.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "ConnectionHandler.hpp"
#include "entities/commsim/Broker.hpp"
//#include "Session.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"

namespace sim_mob {
ConnectionHandler::ConnectionHandler(
		session_ptr session_ ,
//		Broker& broker,
//		messageReceiveCallback callback,
		boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_,
		std::string clientID_,
		unsigned int ClienType_ ,
		unsigned int agentPtr_
		):messageReceiveCallback(messageReceiveCallback_)//theBroker(broker), receiveCallBack(callback)

{
	mySession = session_;
	clientID = clientID_;
	clientType = ClienType_;
	agentPtr = agentPtr_;
//	incomingMessage = "'\0'";
	valid = true;
}

ConnectionHandler::~ConnectionHandler(){
//	mySession.reset();
}
void ConnectionHandler::start()
{


	Json::Value packet;
	Json::Value packet_header = JsonParser::createPacketHeader(pckt_header(1, clientID));
	Json::Value msg = JsonParser::createMessageHeader(msg_header("0","SIMMOBILITY","READY", "SYS"));
	packet["PACKET_HEADER"] = packet_header;
	packet["DATA"].append(msg);//no other data element needed
	std::string readyMessage = Json::FastWriter().write(packet);
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
		messageReceiveCallback(shared_from_this(),incomingMessage);
		//	keep reading
		mySession->async_read(incomingMessage,
						boost::bind(&ConnectionHandler::readHandler, this,
								boost::asio::placeholders::error));
	}
}

void ConnectionHandler::async_send(std::string str)
{
	mySession->async_write(str,boost::bind(&ConnectionHandler::sendHandler, this, boost::asio::placeholders::error));
}
bool ConnectionHandler::send(std::string str) {
	boost::system::error_code ec;
	mySession->write(str, ec);
}
void ConnectionHandler::sendHandler(const boost::system::error_code& e) {
//	Print() << "Write to agent[" << agentPtr << "]  client["  << clientID << "] " <<(e?"Failed":"Success") << std::endl;
}

session_ptr &ConnectionHandler::session(){
	return mySession;
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
