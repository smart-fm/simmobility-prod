/*
 * ConnectionHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "logging/Log.hpp"

namespace sim_mob {

//Forward Declaration
class Broker;
class Session;
class ConnectionHandler;
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler>
{
	boost::shared_ptr<Session> mySession;
	typedef void (Broker::*messageReceiveCallback)(boost::shared_ptr<ConnectionHandler>,std::string);
	messageReceiveCallback receiveCallBack;
	Broker &theBroker;
	std::string incomingMessage;
public:
	//metadata
	//some of such data is duplicated in the broker client list entries
	unsigned int clientType;
	std::string clientID;
	unsigned long int agentPtr;
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(
			boost::shared_ptr<Session> session_ ,
			Broker& broker,
			messageReceiveCallback callback,
			std::string clientID_ = "'\0'",
			unsigned int ClienType_ = 0,
			unsigned int agentPtr_ = 0
			);
	~ConnectionHandler();
	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
	bool is_open();
};//ConnectionHandler

} /* namespace sim_mob */
