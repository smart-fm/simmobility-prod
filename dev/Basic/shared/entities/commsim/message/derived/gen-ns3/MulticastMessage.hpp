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
#include "entities/commsim/message/derived/roadrunner-android/MulticastHandler.hpp"
/*
namespace sim_mob {
class Agent;
class Broker;
class ClientHandler;

namespace comm {


///NS3 multicast message class (no documentation provided).
class NS3_MSG_MULTICAST : public sim_mob::comm::Message {
public:
	NS3_MSG_MULTICAST(const sim_mob::comm::MsgData& data_);
	sim_mob::Handler* newHandler();
};


//Handler for the above message
class NS3_HDL_MULTICAST : public sim_mob::Handler {
public:
	void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller);
};

}}
*/
