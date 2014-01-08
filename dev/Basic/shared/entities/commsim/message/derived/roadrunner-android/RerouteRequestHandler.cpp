//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RerouteRequestHandler.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;


//handler implementation
void sim_mob::roadrunner::RerouteRequestHandler::handle(sim_mob::comm::MsgPtr message_,Broker* broker)
{
	//Just parse it manually.
	sim_mob::comm::MsgData &data = message_->getData();
	Json::Value sender = data.get("SENDER", "UNKNOWN"); //Agent requesting a reroute.
	Json::Value blacklistRegion = data.get("blacklist_region", "UNDFINED"); //Which Region to avoid.

	//TODO: Pass this along to the Agent.
	Print() <<"REROUTE REQUEST IGNORED.\n";
}


