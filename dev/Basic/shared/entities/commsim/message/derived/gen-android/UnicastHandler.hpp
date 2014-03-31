//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "entities/commsim/message/base/Handler.hpp"

namespace sim_mob {
class Agent;
class Broker;
class ClientHandler;

namespace comm {

//Generic handler for ns3+android and android-only activities.
/*class UnicastHandler : public sim_mob::Handler {
public:
	UnicastHandler(bool useNs3);

	void handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller);

private:
	virtual void postProcess(sim_mob::Broker& broker, const sim_mob::Agent& destAgent, sim_mob::ClientHandler& destCliHandler, const std::string andrSensorId, const std::string& andrSensorType, sim_mob::comm::MsgData &data);

private:
	bool useNs3;
};
*/
}}

