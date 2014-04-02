//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AgentCommUtility.hpp"

#include "util/LangHelpers.hpp"

using namespace sim_mob;

sim_mob::AgentCommUtility::AgentCommUtility(sim_mob::Agent* entity_)
	: entity(entity_), broker(nullptr), registered(false), type(0),
	  incomingIsDirty(false), outgoingIsDirty(false), writeIncomingDone(false), readOutgoingDone(false), agentUpdateDone(false)
{
	registrationCallback = &AgentCommUtility::registrationCallBack;
}

void sim_mob::AgentCommUtility::setBroker(sim_mob::Broker *broker)
{
	this->broker = broker;
}

sim_mob::Broker* sim_mob::AgentCommUtility::getBroker()
{
	return broker;
}

void sim_mob::AgentCommUtility::setwriteIncomingDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	writeIncomingDone = value;
}

void sim_mob::AgentCommUtility::setWriteOutgoingDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	readOutgoingDone = value;
}

void sim_mob::AgentCommUtility::setAgentUpdateDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	agentUpdateDone = value;
}

bool sim_mob::AgentCommUtility::iswriteIncomingDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	return writeIncomingDone;
}

bool sim_mob::AgentCommUtility::isreadOutgoingDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	return readOutgoingDone;
}

bool sim_mob::AgentCommUtility::isAgentUpdateDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return agentUpdateDone;
}

bool sim_mob::AgentCommUtility::isOutgoingDirty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	return outgoingIsDirty;
}

bool sim_mob::AgentCommUtility::isIncomingDirty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	return incomingIsDirty;
}

void sim_mob::AgentCommUtility::reset()
{
	{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	agentUpdateDone = false ;
	}
	{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	outgoingIsDirty = false ;
	readOutgoingDone = false ;
	}
	{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	incomingIsDirty = false ;
	writeIncomingDone = false ;
	}
}

void sim_mob::AgentCommUtility::init()
{
}


void sim_mob::AgentCommUtility::RegisterWithBroker()
{
	if(!broker) {
		throw std::runtime_error("Broker Not specified");
	}
	broker->registerEntity(this);
}


void sim_mob::AgentCommUtility::setregistered(bool value)
{
	if(!broker) {
		throw std::runtime_error("Broker Not specified");
	}
	if(registered) {
		throw std::runtime_error("Broker Already Registered-");
	}
	registered = value;
}

//mark the agent as being registered with the given broker
void sim_mob::AgentCommUtility::setregistered(sim_mob::Broker *broker, bool value)
{
	//3.some checks
	if(!broker) {
		throw std::runtime_error("Broker Not specified");
	}

	if(registered) {
		throw std::runtime_error("This Agent has already registered with this Broker");
	}
	//4.now you will remember you have registered with this broker
	registered = value;
}

void sim_mob::AgentCommUtility::registrationCallBack(bool registered)
{
	setregistered(registered);
}

sim_mob::Agent* sim_mob::AgentCommUtility::getEntity()
{
	return entity;
}


