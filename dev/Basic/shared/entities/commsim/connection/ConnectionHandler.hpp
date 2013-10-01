//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ConnectionHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CONNECTIONHANDLER_HPP_
#define CONNECTIONHANDLER_HPP_
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "logging/Log.hpp"
#include "Session.hpp"

namespace sim_mob {
class ConnectionHandler;
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler>
{
	session_ptr mySession;
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback;
	std::string incomingMessage;
	bool valid;
public:
	//metadata
	//some of such data is duplicated in the broker client list entries
	unsigned int clientType;
	std::string clientID;
	unsigned long int agentPtr;
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(
			session_ptr session_ ,
			boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_,
			std::string clientID_ = "'\0'",
			unsigned int ClienType_ = 0,
			unsigned int agentPtr_ = 0
			);
	~ConnectionHandler();
	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void async_send(std::string str);
	bool send(std::string str,boost::system::error_code& e);
	bool send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
	session_ptr &session();
	bool is_open();
	bool isValid();
	void setValidation(bool);

};//ConnectionHandler

} /* namespace sim_mob */
#endif /* CONNECTIONHANDLER_HPP_ */
