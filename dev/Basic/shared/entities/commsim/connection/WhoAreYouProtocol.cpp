//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WhoAreYouProtocol.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include "entities/commsim/Broker.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"

using namespace sim_mob;

void sim_mob::WhoAreYouProtocol::QueryAgentAsync(boost::shared_ptr<sim_mob::ConnectionHandler> conn, BrokerBase& broker)
{
	//We'll need to make a new ConnectionHandler if we don't have an existing one.
	if (!conn) {
		throw std::runtime_error("Cannot QueryAgentAsync() without an existing connection.");
	}

	//Inform the Broker that this connection is waiting on a WHOAMI response.
	broker.insertIntoWaitingOnWHOAMI(conn->getToken(), conn);

	//At this point, we have a ConnectionHandler that can at least receive messages. So send the "WHOAREYOU" request.
	//This will be received by the Broker, and added to the messageReceived() callback, which should then be filtered as expected.
	conn->forwardMessage(CommsimSerializer::makeWhoAreYou(conn->getToken()));
}

