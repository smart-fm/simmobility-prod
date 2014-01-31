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
	///existingConn can be used to multiplex connections. If null, it will be created when "queryAgentAsync()" is called.
	WhoAreYouProtocol(boost::shared_ptr<Session> &sess_, ConnectionServer &, Broker& broker, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn);

	void queryAgentAsync();
private:
	boost::shared_ptr<Session>  sess;
	ConnectionServer &server;
	Broker& broker;
	std::string response; //json string containing ID & type of the client
	boost::shared_ptr<sim_mob::ConnectionHandler> existingConn;
};

}
