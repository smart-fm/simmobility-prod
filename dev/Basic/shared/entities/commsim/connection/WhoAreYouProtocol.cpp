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
#include "ConnectionServer.hpp"
#include "Session.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"
#include "entities/commsim/Broker.hpp"
#include "logging/Log.hpp"

namespace sim_mob {

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr &sess_, ConnectionServer &server_, Broker& broker, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn)
	: sess(sess_), server(server_), broker(broker), existingConn(existingConn)
{
}

void WhoAreYouProtocol::queryAgentAsync()
{
	//We'll need to make a new ConnectionHandler if we don't have an existing one.
	if (!existingConn) {
		existingConn.reset(new ConnectionHandler(sess, broker.getMessageReceiveCallBack()));
	}

	//Inform the Broker about this session.
	broker.insertIntoWaitingOnWHOAMI(existingConn);

	//At this point, we have a ConnectionHandler that can at least receive messages. So send the "WHOAREYOU" request.
	//This will be received by the Broker, and added to the messageReceived() callback, which should then be filtered as expected.
	existingConn->forwardMessage(JsonParser::makeWhoAreYouPacket());
}


} /* namespace sim_mob */
