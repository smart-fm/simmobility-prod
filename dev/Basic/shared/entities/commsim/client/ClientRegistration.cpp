//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "ClientRegistration.hpp"

#include "util/LangHelpers.hpp"

#include "entities/Person.hpp"
#include "entities/commsim/client/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "event/EventPublisher.hpp"

using namespace sim_mob;



boost::shared_ptr<ClientHandler> sim_mob::ClientRegistrationHandler::makeClientHandler(boost::shared_ptr<sim_mob::ConnectionHandler> connHandle, BrokerBase& broker, ClientRegistrationRequest& request, const Agent* freeAgent, bool isNs3Client)
{
	//Create a ClientHandler pointing to the Broker.
	boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker, connHandle,  freeAgent, request.clientID));
	clientEntry->setRequiredServices(request.requiredServices);

	//Subscribe to relevant services for each required service.
	for (std::set<sim_mob::Services::SIM_MOB_SERVICE>::const_iterator it=request.requiredServices.begin(); it!=request.requiredServices.end(); it++) {
		switch (*it) {
			case sim_mob::Services::SIMMOB_SRV_TIME:
				clientEntry->regisTime = true;
				break;
			case sim_mob::Services::SIMMOB_SRV_LOCATION:
				clientEntry->regisLocation = true;
				break;
			case sim_mob::Services::SIMMOB_SRV_REGIONS_AND_PATH:
				clientEntry->regisRegionPath = true;
				break;
			case sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS:
				clientEntry->regisAllLocations = true;
				break;
			default:
				Warn() <<"Client requested service which could not be provided.\n"; break;
		}
	}


	//also, add the client entry to broker(for message handler purposes)
	broker.insertClientList(request.clientID, (isNs3Client?Broker::ClientTypeNs3:Broker::ClientTypeAndroid), clientEntry);

	//add this agent to the list of the agents who are associated with a android emulator client
	if (freeAgent) {
		usedAgents.insert(freeAgent);
	}

	return clientEntry;
}

bool sim_mob::ClientRegistrationHandler::handle(BrokerBase& broker, sim_mob::ClientRegistrationRequest& request, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn, bool isNs3Client)
{
	//some checks to avoid calling this method unnecessarily
	//TODO: It seems like this check is actually not mandatory; I'd like to remove it. ~Seth
	//if (broker.getClientWaitingListSize()==0) {
	//	return false;
	//}

	//Android clients require a free agent to associate with.
	const Agent* agent = isNs3Client ? nullptr : findAFreeAgent(broker.getRegisteredAgents());
	if (!isNs3Client && !agent) {
		return false;
	}

	//use it to create a client entry
	boost::shared_ptr<ClientHandler> clientEntry = makeClientHandler(existingConn, broker, request, agent, isNs3Client);

	//AgentsInfo is required before READY
	if (isNs3Client) {
		sendAgentsInfo(broker.getRegisteredAgents(), clientEntry);
	}

	//Inform the client we are ready to proceed.
	clientEntry->connHandle->forwardReadyMessage(*clientEntry);

	return true;
}







void sim_mob::ClientRegistrationHandler::sendAgentsInfo(const std::map<const Agent*, AgentInfo>& agents, boost::shared_ptr<ClientHandler> clientEntry)
{
	//send some initial configuration information to NS3
	std::vector<unsigned int> keys;

	//please mind the AgentInfo vs AgentsInfo
	for (std::map<const Agent*, AgentInfo>::const_iterator it = agents.begin(); it != agents.end(); it++) {
		keys.push_back(it->first->getId());
	}

	//We are cheating a bit here.
	StartTimePriorityQueue pending(Agent::pending_agents);
	while (!pending.empty()) {
		Person* p = dynamic_cast<Person*>(pending.top());
		if (p) {
			keys.push_back(p->getId());
		}
		pending.pop();
	}

	OngoingSerialization ongoing;
	CommsimSerializer::serialize_begin(ongoing, boost::lexical_cast<std::string>(clientEntry->clientId));
	CommsimSerializer::addGeneric(ongoing, CommsimSerializer::makeAgentsInfo(keys, std::vector<unsigned int>()));

	BundleHeader hRes;
	std::string msg;
	CommsimSerializer::serialize_end(ongoing, hRes, msg);
	clientEntry->connHandle->forwardMessage(hRes, msg);
}


const Agent* sim_mob::ClientRegistrationHandler::findAFreeAgent(const std::map<const Agent*, AgentInfo>& registeredAgents)
{
	for (std::map<const Agent*, AgentInfo>::const_iterator it=registeredAgents.begin(); it!=registeredAgents.end(); it++) {
		if (usedAgents.find(it->first) == usedAgents.end()) {
			return it->first;
		}
	}

	return nullptr;
}

