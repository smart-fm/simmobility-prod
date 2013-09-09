#pragma once
#include "entities/commsim/buffer/BufferContainer.hpp"
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
namespace sim_mob
{
//class Broker;
class Agent;

class AgentCommUtilityBase {
private:

	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
	bool incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
	bool outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)

	bool writeIncomingDone;//the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
	bool readOutgoingDone;//the 'communicator' agent  read the outgoing buffer of the 'communicating' agent
	bool agentUpdateDone;
	//todo make this an enum-vahid
	std::string agentIdentification;
	sim_mob::Agent *entity;//the entity this class is actually referring to
	//future use:
	unsigned int type;//do you want to register as a pedestrian, driver,....

protected:
	sim_mob::Broker * communicator;

public:
	bool registered;
	void (AgentCommUtilityBase::*registrationCallback)(bool);

	boost::shared_mutex mutex;
	boost::shared_mutex mutex_outgoing;
	boost::shared_mutex mutex_incoming;
	AgentCommUtilityBase(/*sim_mob::Broker* managingBroker, */sim_mob::Agent* entity_):
 	entity(entity_),
	communicator(nullptr),
	incomingIsDirty(false),
	outgoingIsDirty(false),
	writeIncomingDone(false),
	readOutgoingDone(false),
	agentUpdateDone(false),
	type(0)
	{
		registered = false;
		registrationCallback = &AgentCommUtilityBase::registrationCallBack;
	}

	void setBroker(sim_mob::Broker *broker){
//		Print() << "Setting the communicator to [" << broker << "]" << std::endl;
		communicator = broker;
	}

	sim_mob::Broker *getBroker()
	{
		return communicator;
	}

	void setwriteIncomingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		writeIncomingDone = value;
	}

	void setWriteOutgoingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
		readOutgoingDone = value;
	}

	void setAgentUpdateDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(mutex);
//		communicator->setAgentUpdateDone(entity);
		agentUpdateDone = value;
	}

	bool &iswriteIncomingDone() {
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		return writeIncomingDone;
	}

	bool &isreadOutgoingDone() {
		boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
		return readOutgoingDone;
	}

	bool &isAgentUpdateDone() {
		boost::unique_lock< boost::shared_mutex > lock(mutex);
			return agentUpdateDone;
	}


	bool &isOutgoingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
		return outgoingIsDirty;
	}

	bool &isIncomingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		return incomingIsDirty;
	}

//todo

	void reset(){
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

	void init(){

	}

	//this is used to register the drived class
	//(which is also an agent) to the communicator agent

	bool RegisterWithBroker(/*sim_mob::Agent* registerr, sim_mob::Broker *communicator_*/)
	{

//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(registerr);
//		communicator = communicator_;
//		Print() << "AgentCommUtility::Registering With Broker[" << communicator << "]" << std::endl;
		if(!communicator)
		{
			throw std::runtime_error("Broker Not specified");
		}
		return communicator->registerEntity(this);
//		std::cout << "agent[" << &getEntity() << "] was registered with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	//	subscriptionInfo getSubscriptionInfo();
		void setregistered(bool value)
		{
			registered = value;
		};

	void registrationCallBack(bool registered)
	{
		setregistered(registered);
	}

	sim_mob::Agent* getEntity()
	{
		return entity;
	}

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
	AgentCommUtility(sim_mob::Agent* entity_):
		AgentCommUtilityBase(entity_)
{
}
;
	virtual ~AgentCommUtility(){};

private:
	BufferContainer<MSG_TYPE> incoming;
	BufferContainer<MSG_TYPE> outgoing;

public:
	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	BufferContainer<MSG_TYPE>& getIncoming() {
		return incoming;
	};
	void getAndClearIncoming(BufferContainer<MSG_TYPE> &values) {
		values = incoming;
		incoming.clear();
	};
	BufferContainer<MSG_TYPE>& getOutgoing(){
		boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
		return outgoing;
	};
	void setIncoming(BufferContainer<MSG_TYPE> values) {
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		incoming = values;
	};
	bool popIncoming(MSG_TYPE &var)
	{
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		return incoming.pop(var);
	};

void addIncoming(MSG_TYPE value) {
		boost::unique_lock< boost::shared_mutex > lock(mutex_incoming);
		incoming.add(value);
		incomingIsDirty = true;
	}

	void addOutgoing(MSG_TYPE value) {
		boost::unique_lock< boost::shared_mutex > lock(mutex_outgoing);
		outgoing.add(value);
		outgoingIsDirty = true;
	}

};//end of class  AgentCommUtility
};//namespace
