//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NewClientHandler.hpp"

#include "entities/commsim/connection/WhoAreYouProtocol.hpp"
#include "entities/commsim/Broker.hpp"

using namespace sim_mob;


//handler implementation
void sim_mob::roadrunner::NewClientHandler::handle(sim_mob::comm::MsgPtr message_, sim_mob::Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller)
{
	//Query this agent's ID; tell the Broker that we are expecting a reply.
	WhoAreYouProtocol::QueryAgentAsync(caller, *broker);
}
