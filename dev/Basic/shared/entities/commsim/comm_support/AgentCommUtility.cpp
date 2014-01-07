//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AgentCommUtility.hpp"

#include "util/LangHelpers.hpp"

using namespace sim_mob;

sim_mob::AgentCommUtilityBase::AgentCommUtilityBase(sim_mob::Agent* entity_)
	: entity(entity_), incomingIsDirty(false), outgoingIsDirty(false),
	  writeIncomingDone(false), readOutgoingDone(false), agentUpdateDone(false), type(0)
{
	registrationCallback = &AgentCommUtilityBase::registrationCallBack;
}

void sim_mob::AgentCommUtilityBase::setBroker(const std::string& commElement,sim_mob::Broker *broker)
{
	brokers[commElement] = broker;
}

sim_mob::Broker* sim_mob::AgentCommUtilityBase::getBroker(const std::string & commElement)
{
	if(brokers.find(commElement) != brokers.end()){
		return brokers[commElement];
	}
	return nullptr;
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


bool sim_mob::AgentCommUtilityBase::RegisterWithBroker(const std::string& commElement)
{

//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(registerr);
//		communicator = communicator_;
//		Print() << "AgentCommUtility::Registering With Broker[" << communicator << "]" << std::endl;
	std::map<const std::string,sim_mob::Broker*>::iterator it = brokers.find(commElement);
	if(it == brokers.end()) {
		throw std::runtime_error("Broker Not specified");
	}
	sim_mob::Broker * broker = it->second;
	return broker->registerEntity(this);
//		std::cout << "agent[" << &getEntity() << "] was registered with outgoing[" << &(getOutgoing()) << "]" << std::endl;
}


void sim_mob::AgentCommUtilityBase::setregistered(std::string commElement, bool value)
{

	if(brokers.find(commElement) == brokers.end()) {
		throw std::runtime_error("Broker Not specified");
	}
	if(registered.find(commElement) != registered.end()) {
		throw std::runtime_error("Broker Already Registered-");
	}
	registered[commElement] = value;
}

//mark the agent as being registered with the given broker
void sim_mob::AgentCommUtilityBase::setregistered(sim_mob::Broker *broker, bool value)
{
	//1- find the broker in the list
	sim_mob::Broker *tempBroker = nullptr;
	std::string commElement;
	std::map<const std::string,sim_mob::Broker*>::const_iterator it(brokers.begin()), it_end(brokers.end());
	for(; it != it_end; it++){
		if(it->second == broker)
		{
			tempBroker = broker;
			//2.get the broker name(application actually: roadrunner, stk, etc)
			commElement = it->first;
			break;
		}
	}
	//3.some checks
	if(!tempBroker) {
		throw std::runtime_error("Broker Not specified");
	}

	if(registered.find(commElement) != registered.end()) {
		throw std::runtime_error("This Agent has already registered with this Broker");
	}
	//4.now you will remember you have registered with this broker
	registered[commElement] = value;
}

void sim_mob::AgentCommUtilityBase::registrationCallBack(std::string commElement, bool registered)
{
	setregistered(commElement,registered);
}

sim_mob::Agent* sim_mob::AgentCommUtilityBase::getEntity()
{
	return entity;
}


