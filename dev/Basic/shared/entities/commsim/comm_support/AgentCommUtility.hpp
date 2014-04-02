//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/broker/Broker.hpp"


//this file provides some of the communication
//1-tools required for agent "communication subsystem"
//2-buffers used by agent for their incoming and outgoing data


/*********************************************************************************
 communication subsystem:
 the basic idea is to provide send and receive functionality.
 This involves :
 1-defining send and receive arguments and return value types
 2-Implementing send and receive based on the current requirement, situation and scenario etc.
 3-used by communicator agent only
 **********************************************************************************/



//using namespace sim_mob;
namespace sim_mob {
class Agent;

/*********************************************************************************
 * Any agent who wants to send or receive data, should inherit this class ("AgentCommUtility") also
 * it gives you buffers :
 * contains the following:
 * 1- Shared items for incoming and out going of data
 * the following data structure is held/kept by the agents who would like to communicate with other agents
 * So the agent communicator will read from 'outgoing' of the agents and also writes into their incoming
 *
 * ofcourse, the following are used by agents other than communicator agents
 *********************************************************************************/
class AgentCommUtility {
public:
	AgentCommUtility(sim_mob::Agent* entity_);

	void (AgentCommUtility::*registrationCallback)(bool);

	void setBroker(sim_mob::Broker *broker);
	sim_mob::Broker *getBroker();

	void setwriteIncomingDone(bool value);

	void setWriteOutgoingDone(bool value);

	void setAgentUpdateDone(bool value);

	//NOTE: I am making these return bool instead of bool&  --returning a reference
	//      like this is fine, but since you are locking the value I assume it is an error
	//      (since *using* the returned value will not be thread-safe). ~Seth
	bool iswriteIncomingDone();
	bool isreadOutgoingDone();
	bool isAgentUpdateDone();
	bool isOutgoingDirty();
	bool isIncomingDirty();

//todo
	void reset();

	void init();

	//this is used to register the drived class
	//(which is also an agent) to the communicator agent
	void RegisterWithBroker();

	//	subscriptionInfo getSubscriptionInfo();
	void setregistered(bool value);
	void setregistered(sim_mob::Broker *broker, bool value);

	void registrationCallBack(bool registered);

	sim_mob::Agent* getEntity();

private:
	boost::shared_mutex mutex;
	boost::shared_mutex mutex_outgoing;
	boost::shared_mutex mutex_incoming;

	sim_mob::Agent *entity;//the entity this class is actually referring to
	Broker* broker;
	bool registered;

	//future use:
	unsigned int type;//do you want to register as a pedestrian, driver,....

	bool writeIncomingDone;//the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
	bool readOutgoingDone;//the 'communicator' agent  read the outgoing buffer of the 'communicating' agent
	bool agentUpdateDone;
	//todo make this an enum-vahid
	std::string agentIdentification;

	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
	bool incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
	bool outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)

};


}



