//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RerouteRequestHandler.hpp"

#include "logging/Log.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "event/args/ReRouteEventArgs.hpp"

using namespace sim_mob;


//handler implementation
/*void sim_mob::roadrunner::RerouteRequestHandler::handle(sim_mob::comm::MsgPtr message_,Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller)
{
	//Just parse it manually.
	sim_mob::comm::MsgData &data = message_->getData();
	Json::Value sender = data.get("SENDER", "UNKNOWN"); //Agent requesting a reroute.
	Json::Value senderType = data.get("SENDER_TYPE", "UNDEFINED"); //Usually "SIMMOBILITY"
	Json::Value blacklistRegion = data.get("blacklist_region", "UNDFINED"); //Which Region to avoid.

	boost::shared_ptr<sim_mob::ClientHandler> agentHandle;
	if(!broker->getClientHandler(sender.asString(), senderType.asString(), agentHandle)) {
		WarnOut("RerouteRequest can't find Agent (self)." << std::endl);
		return;
	}

	//Double-check Agent validity.
	if(!agentHandle->agent) {
		WarnOut( "RerouteRequest found invalid Agent; may have completed route already." << std::endl);
		return;
	}

	//Now dispatch through the MessageBus.
	sim_mob::messaging::MessageBus::PublishEvent(sim_mob::event::EVT_CORE_COMMSIM_REROUTING_REQUEST,
			agentHandle->agent, messaging::MessageBus::EventArgsPtr(new event::ReRouteEventArgs(blacklistRegion.asString())));
}
*/

