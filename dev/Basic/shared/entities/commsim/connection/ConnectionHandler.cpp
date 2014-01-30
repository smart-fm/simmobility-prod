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
#include "entities/commsim/serialization/JsonParser.hpp"

using namespace sim_mob;

sim_mob::ConnectionHandler::ConnectionHandler(session_ptr session, boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_, std::string clientID_, sim_mob::comm::ClientType clientType)
	: messageReceiveCallback(messageReceiveCallback_), clientType(clientType), session(session), clientId(clientId), valid(true)
{
}


void sim_mob::ConnectionHandler::start()
{
	Json::Value packet;
	Json::Value packet_header = JsonParser::createPacketHeader(pckt_header(1, clientId));
	Json::Value msg = JsonParser::createMessageHeader(msg_header("0","SIMMOBILITY","READY", "SYS"));
	packet["PACKET_HEADER"] = packet_header;
	packet["DATA"].append(msg);//no other data element needed
	std::string readyMessage = Json::FastWriter().write(packet);
	session->async_write(readyMessage,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,readyMessage));
}



void sim_mob::ConnectionHandler::readyHandler(const boost::system::error_code &e, std::string str)
{
	if(e)
	{
		std::cerr << "Connection Not Ready[" << e.message() << "] Trying Again" << std::endl;
		session->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
	}
	else
	{
		//will not pass 'message' variable as argument coz it
		//is global between functions. some function read into it, another function read from it
		session->async_read(incomingMessage,
			boost::bind(&ConnectionHandler::readHandler, this,
					boost::asio::placeholders::error));

	}
}

void sim_mob::ConnectionHandler::readHandler(const boost::system::error_code& e)
{

	if(e)
	{
		std::cerr << "Read Fail [" << e.message() << "]" << std::endl;
	}
	else
	{
		//call the receive handler in the broker
		messageReceiveCallback(shared_from_this(),incomingMessage);
		//	keep reading
		session->async_read(incomingMessage,
			boost::bind(&ConnectionHandler::readHandler, this,
			boost::asio::placeholders::error)
		);
	}
}

void sim_mob::ConnectionHandler::async_send(std::string str)
{
	session->async_write(str,boost::bind(&ConnectionHandler::sendHandler, this, boost::asio::placeholders::error));
}
void sim_mob::ConnectionHandler::send(std::string str)
{
	boost::system::error_code ec;
	session->write(str, ec);
}

void sim_mob::ConnectionHandler::sendHandler(const boost::system::error_code& e)
{
//	Print() << "Write to agent[" << agentPtr << "]  client["  << clientID << "] " <<(e?"Failed":"Success") << std::endl;
}

session_ptr& sim_mob::ConnectionHandler::getSession()
{
	return session;
}

bool sim_mob::ConnectionHandler::is_open()
{
	return session->socket().is_open();
}


bool sim_mob::ConnectionHandler::isValid()
{
	return valid;
}

void ConnectionHandler::setValidation(bool value)
{
	valid = value;
}

sim_mob::comm::ClientType sim_mob::ConnectionHandler::getClientType() const
{
	return clientType;
}
