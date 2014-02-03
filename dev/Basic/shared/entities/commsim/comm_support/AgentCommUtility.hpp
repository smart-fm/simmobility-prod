//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/buffer/BufferContainer.hpp"
#include "entities/commsim/Broker.hpp"


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

class AgentCommUtilityBase {
protected:
	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
	bool incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
	bool outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)

private:

	bool writeIncomingDone;//the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
	bool readOutgoingDone;//the 'communicator' agent  read the outgoing buffer of the 'communicating' agent
	bool agentUpdateDone;
	//todo make this an enum-vahid
	std::string agentIdentification;
	sim_mob::Agent *entity;//the entity this class is actually referring to
	//future use:
	unsigned int type;//do you want to register as a pedestrian, driver,....

protected:
	std::map<const std::string,sim_mob::Broker*> brokers;

public:
	std::map<const std::string,bool> registered;
	void (AgentCommUtilityBase::*registrationCallback)(std::string ,bool);

	boost::shared_mutex mutex;
	boost::shared_mutex mutex_outgoing;
	boost::shared_mutex mutex_incoming;

	AgentCommUtilityBase(sim_mob::Agent* entity_);

	void setBroker(const std::string& commElement,sim_mob::Broker *broker);

	sim_mob::Broker *getBroker(const std::string& commElement);

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
	void RegisterWithBroker(const std::string& commElement);

	//	subscriptionInfo getSubscriptionInfo();
	void setregistered(std::string commElement,bool value);
	void setregistered(sim_mob::Broker *broker, bool value);

	void registrationCallBack(std::string commElement, bool registered);

	sim_mob::Agent* getEntity();

};

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

/*subscriptionInfo*/;
//this class will be inherited by any agent who would like participate in agent communication
//The normal method for a communicating driver, for example, is:
//create a class inherited from driver and the the following class.
//then override the driver's(agent's) update method to use send & receive and set
//various flags as necessary.
template<class MSG_TYPE>
class AgentCommUtility :public AgentCommUtilityBase{
public:
	AgentCommUtility(sim_mob::Agent* entity_);

	virtual ~AgentCommUtility();

private:
	BufferContainer<MSG_TYPE> incoming;
	BufferContainer<MSG_TYPE> outgoing;

public:
	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	BufferContainer<MSG_TYPE>& getIncoming();
	void getAndClearIncoming(BufferContainer<MSG_TYPE> &values);
	BufferContainer<MSG_TYPE>& getOutgoing();
	void setIncoming(BufferContainer<MSG_TYPE> values);
	bool popIncoming(MSG_TYPE &var);

	void addIncoming(MSG_TYPE value);

	void addOutgoing(MSG_TYPE value);

};//end of class  AgentCommUtility

} //End sim_mob namespace


//////////////////////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////////////////////

template <class MSG_TYPE>
sim_mob::AgentCommUtility<MSG_TYPE>::AgentCommUtility(sim_mob::Agent* entity_) : AgentCommUtilityBase(entity_)
{}

template <class MSG_TYPE>
sim_mob::AgentCommUtility<MSG_TYPE>::~AgentCommUtility()
{}

template <class MSG_TYPE>
sim_mob::BufferContainer<MSG_TYPE>& sim_mob::AgentCommUtility<MSG_TYPE>::getIncoming()
{
	return incoming;
}

template <class MSG_TYPE>
void sim_mob::AgentCommUtility<MSG_TYPE>::getAndClearIncoming(BufferContainer<MSG_TYPE> &values)
{
	values = incoming;
	incoming.clear();
}

template <class MSG_TYPE>
sim_mob::BufferContainer<MSG_TYPE>& sim_mob::AgentCommUtility<MSG_TYPE>::getOutgoing()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	return outgoing;
}

template <class MSG_TYPE>
void sim_mob::AgentCommUtility<MSG_TYPE>::setIncoming(BufferContainer<MSG_TYPE> values)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	incoming = values;
}

template <class MSG_TYPE>
bool sim_mob::AgentCommUtility<MSG_TYPE>::popIncoming(MSG_TYPE &var)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	return incoming.pop(var);
}

template <class MSG_TYPE>
void sim_mob::AgentCommUtility<MSG_TYPE>::addIncoming(MSG_TYPE value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	incoming.add(value);
	incomingIsDirty = true;
}

template <class MSG_TYPE>
void sim_mob::AgentCommUtility<MSG_TYPE>::addOutgoing(MSG_TYPE value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	outgoing.add(value);
	outgoingIsDirty = true;
}
