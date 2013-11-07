//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MULTICAST_Message.h
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"
#include "entities/commsim/message/base/MulticastHandler.hpp"

namespace sim_mob {
class Agent;
class Broker;
class ClientHandler;

namespace roadrunner {

class MSG_MULTICAST : public sim_mob::comm::Message {
public:
	MSG_MULTICAST(sim_mob::comm::MsgData data_);
	sim_mob::Handler* newHandler();
};


//Handler for the above message
/*class HDL_MULTICAST : public sim_mob::roadrunner::MulticastHandler {
protected:
	//Called whenever a client is found that we must dispatch a message to.
	//Behavior differs for ns3 versus android-only.
	virtual void handleClient(const sim_mob::ClientHandler& clientHdlr, sim_mob::comm::MsgData& recipientsList, sim_mob::Broker& broker, sim_mob::comm::MsgData& data);

	//Called when all client have been processed and messages may now be sent.
	//Behavior only exists for ns-3 (where messages are delayed).
	virtual void postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const sim_mob::comm::MsgData& recipientsList, sim_mob::comm::MsgData& data);
};*/

}}
