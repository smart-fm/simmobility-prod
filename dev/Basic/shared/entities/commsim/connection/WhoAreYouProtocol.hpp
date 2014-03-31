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
class BrokerBase;
class Session;
class ConnectionHandler;

class WhoAreYouProtocol
{
public:
	//Begin the query process for agents. conn must be non-null, but you can re-use an existing conn if required.
	static void QueryAgentAsync(boost::shared_ptr<sim_mob::ConnectionHandler> conn, BrokerBase& broker);
};

}
