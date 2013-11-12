//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AgentCommUtility.hpp"

#include "util/LangHelpers.hpp"

using namespace sim_mob;

sim_mob::AgentCommUtilityBase::AgentCommUtilityBase(sim_mob::Agent* entity_)
	: entity(entity_), communicator(nullptr), incomingIsDirty(false), outgoingIsDirty(false),
	  writeIncomingDone(false), readOutgoingDone(false), agentUpdateDone(false), type(0)
{
	registered = false;
	registrationCallback = &AgentCommUtilityBase::registrationCallBack;
}

void sim_mob::AgentCommUtilityBase::setBroker(sim_mob::Broker *broker)
{
	communicator = broker;
}

sim_mob::Broker* sim_mob::AgentCommUtilityBase::getBroker()
{
	return communicator;
}

void sim_mob::AgentCommUtilityBase::setwriteIncomingDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	writeIncomingDone = value;
}

void sim_mob::AgentCommUtilityBase::setWriteOutgoingDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	readOutgoingDone = value;
}

void sim_mob::AgentCommUtilityBase::setAgentUpdateDone(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	agentUpdateDone = value;
}

bool sim_mob::AgentCommUtilityBase::iswriteIncomingDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	return writeIncomingDone;
}

bool sim_mob::AgentCommUtilityBase::isreadOutgoingDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	return readOutgoingDone;
}

bool sim_mob::AgentCommUtilityBase::isAgentUpdateDone()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex);
	return agentUpdateDone;
}

bool sim_mob::AgentCommUtilityBase::isOutgoingDirty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
	return outgoingIsDirty;
}

bool sim_mob::AgentCommUtilityBase::isIncomingDirty()
{
	boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
	return incomingIsDirty;
}

void sim_mob::AgentCommUtilityBase::reset()
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

void sim_mob::AgentCommUtilityBase::init()
{
}


bool sim_mob::AgentCommUtilityBase::RegisterWithBroker()
{

//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(registerr);
//		communicator = communicator_;
//		Print() << "AgentCommUtility::Registering With Broker[" << communicator << "]" << std::endl;
	if(!communicator) {
		throw std::runtime_error("Broker Not specified");
	}
	return communicator->registerEntity(this);
//		std::cout << "agent[" << &getEntity() << "] was registered with outgoing[" << &(getOutgoing()) << "]" << std::endl;
}


void sim_mob::AgentCommUtilityBase::setregistered(bool value)
{
	registered = value;
}

void sim_mob::AgentCommUtilityBase::registrationCallBack(bool registered)
{
	setregistered(registered);
}

sim_mob::Agent* sim_mob::AgentCommUtilityBase::getEntity()
{
	return entity;
}


