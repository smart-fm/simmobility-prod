//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 *      multicast messages come from android clients(only) and
 *      are supposed to be routed to ns3
 */

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
class Agent;
class Broker;
class ClientHandler;
class MulticastMessage;

namespace roadrunner {

///Shared functionality for both the AndroidMulticastHandler and the AndroidNs3MulticastHandler
class MulticastHandler : public sim_mob::Handler {
public:
	MulticastHandler(bool useNs3);

	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;

private:
	//Called whenever a client is found that we must dispatch a message to.
	//Behavior differs for ns3 versus android-only.
	//TODO: The client handler can't really be const, since we are expecting to respond to this messsage at some point (which may modify the client).
	virtual void handleClient(const sim_mob::ClientHandler& clientHdlr, std::vector<unsigned int>& receiveAgentIds, sim_mob::Broker& broker, MulticastMessage& currMsg);

	//Called when all client have been processed and messages may now be sent.
	//Behavior only exists for ns-3 (where messages are delayed).
	virtual void postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const std::vector<unsigned int>& receiveAgentIds, MulticastMessage& currMsg);

private:
	bool useNs3;
};

}}
