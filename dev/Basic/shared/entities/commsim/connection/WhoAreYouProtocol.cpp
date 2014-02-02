//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WhoAreYouProtocol.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "WhoAreYouProtocol.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"
#include "entities/commsim/Broker.hpp"

namespace sim_mob {

void WhoAreYouProtocol::QueryAgentAsync(boost::shared_ptr<sim_mob::ConnectionHandler> conn, Broker& broker)
{
	//We'll need to make a new ConnectionHandler if we don't have an existing one.
	if (!conn) {
		throw std::runtime_error("Cannot QueryAgentAsync() without an existing connection.");
	}

	//Inform the Broker that this connection is waiting on a WHOAMI response.
	broker.insertIntoWaitingOnWHOAMI(conn);

Warn() <<"Forwarding whoareyou\n";

	//At this point, we have a ConnectionHandler that can at least receive messages. So send the "WHOAREYOU" request.
	//This will be received by the Broker, and added to the messageReceived() callback, which should then be filtered as expected.
	conn->forwardMessage(JsonParser::makeWhoAreYouPacket());
}


} /* namespace sim_mob */
