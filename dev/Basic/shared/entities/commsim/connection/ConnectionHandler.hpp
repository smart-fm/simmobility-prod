//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include "entities/commsim/connection/Session.hpp"
#include "entities/commsim/client/ClientType.hpp"

namespace sim_mob {
class ConnectionHandler;
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler> {
public:
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(session_ptr session , boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback_,
			std::string clientId = "'\0'", sim_mob::comm::ClientType clientType = sim_mob::comm::UNKNOWN_CLIENT);

	//metadata
	//some of such data is duplicated in the broker client list entries
	//
	std::string clientId;

	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void async_send(std::string str);
	void send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
	session_ptr& getSession();
	bool is_open();
	bool isValid();
	void setValidation(bool);
	sim_mob::comm::ClientType getClientType() const;

private:
	//What type of clients (Android, NS3) can this ConnectionHandler manage?
	sim_mob::comm::ClientType clientType;

	session_ptr session;
	boost::function<void(boost::shared_ptr<ConnectionHandler>, std::string)> messageReceiveCallback;
	std::string incomingMessage;
	bool valid;
};//ConnectionHandler

} /* namespace sim_mob */
