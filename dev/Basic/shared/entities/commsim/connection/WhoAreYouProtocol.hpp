//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WhoAreYouProtocol.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#pragma once

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include <entities/commsim/client/base/ClientRegistration.hpp>

namespace sim_mob {
class Session;
class ConnectionServer;
class ConnectionHandler;

class WhoAreYouProtocol
{
public:
	///uniqueSocket is true if this socket is being used for the first time (it causes the Broker to add a listener).
	WhoAreYouProtocol(boost::shared_ptr<Session> &sess_, ConnectionServer &, Broker& broker, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);

	void queryAgentAsync();
private:
	boost::shared_ptr<Session>  sess;
	ConnectionServer &server;
	Broker& broker;
	std::string response; //json string containing ID & type of the client
	boost::shared_ptr<sim_mob::ConnectionHandler> existingConn;

	//sim_mob::ClientRegistrationRequest getSubscriptionRequest(std::string);

	//void WhoAreYou_handler(const boost::system::error_code& e);

	//TODO: Migrate this elsewhere.
	//void WhoAreYou_response_handler(const boost::system::error_code& e);
};

} /* namespace sim_mob */
