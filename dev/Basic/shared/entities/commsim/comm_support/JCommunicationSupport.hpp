#pragma once
#include "entities/commsim/communicator/buffer/BufferContainer.hpp"


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
class Broker;
class Agent;

/*********************************************************************************
 * Any agent who wants to send or receive data, should inherit this class ("JCommunicationSupport") also
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

class JCommunicationSupport {
public:
	JCommunicationSupport(sim_mob::Broker& managingBroker, sim_mob::Agent& entity_);
	virtual ~JCommunicationSupport() {}

private:
	BufferContainer incoming;
	BufferContainer &outgoing;

	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
	bool incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
	bool outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)

	bool writeIncomingDone;//the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
	bool readOutgoingDone;//the 'communicator' agent  read the outgoing buffer of the 'communicating' agent
	bool agentUpdateDone;
	//todo make this an enum-vahid
	std::string agentIdentification;
	sim_mob::Agent &entity;//the entity this class is actually referring to
	//future use:
	unsigned int type;//do you want to subscribe as a pedestrian, driver,....

protected:
	sim_mob::Broker& communicator;


public:
	bool subscribed;
	void (JCommunicationSupport::*subscriptionCallback)(bool);
	//general purpose counter
	int cnt_1;//i use this one to control/limit the number of times communicator faces the 'update not done'
	int cnt_2;
	boost::shared_mutex CommSupp_Mutex;
	std::vector<boost::shared_ptr<boost::shared_mutex> > Broker_Mutexes;
//	subscriptionInfo getSubscriptionInfo();
	void setSubscribed(bool);
	void setMutexes(std::vector<boost::shared_ptr<boost::shared_mutex> > &value);
	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	BufferContainer& getIncoming();
	void getAndClearIncoming(BufferContainer &values);
	BufferContainer& getOutgoing();
	void setIncoming(BufferContainer value);
	bool popIncoming(DataElement &var);
//	void setOutgoing(BufferContainer value); we are now writing directly to communicator buffer so this function is dangerous
	void addIncoming(DataElement value);
	void addOutgoing(DataElement value);

	void setwriteIncomingDone(bool value);
	void setWriteOutgoingDone(bool value);
	void setAgentUpdateDone(bool value);
	bool &iswriteIncomingDone();
	bool &isreadOutgoingDone();
	bool &isAgentUpdateDone();
	bool &isOutgoingDirty();
	bool &isIncomingDirty();


	void init();
	void reset();
	//this is used to subscribe the drived class
	//(which is also an agent) to the communicator agent
	virtual bool subscribe(sim_mob::Agent* subscriber,sim_mob::Broker &communicator);
	virtual const sim_mob::Agent& getEntity();
};

};//namespace
