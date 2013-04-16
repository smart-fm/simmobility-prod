#pragma once
#include "Communication.hpp"
#include "entities/Agent.hpp"
/*******************************************************************************************
 *
 * This agent is the interface between NS3 simulator and other agents
 * he is responsible for sending and receiving outgoing and incoming messages
 * of the agents who have subscribed to it.
 *
 ********************************************************************************************/
namespace sim_mob {
class CommunicationSupport;
class NS3_Communicator : public sim_mob::Agent
{

	bool enabled;

	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&> subscriptionList;
	sim_mob::DataContainer sendBuffer;
	sim_mob::DataContainer trySendBuffer;//send the buffers in batches
	sim_mob::DataContainer receiveBuffer;
	sim_mob::NS3_Communication commImpl;
	static NS3_Communicator instance;
	//for internal use (yeah, other variables are for external use 'only'! ...pinhead)
	std::set<const sim_mob::Entity*> duplicateEntityDoneChecker ;

    //serialize, deserialize, send, receive
	void trySend(timeslice);
	bool processOutgoingData(timeslice now);
	void processIncomingData(timeslice now);
	void reset(); //clear buffers and reset flags
	void reset(sim_mob::CommunicationSupport &info);
	void printSubscriptionList(timeslice now);
	bool deadEntityCheck(sim_mob::CommunicationSupport & info);
	void refineSubscriptionList();
	//threadfunctions
	void bufferSend(DataContainer &tempSendBuffer);
	bool allAgentUpdatesDone();

public:
	boost::shared_mutex *NS3_Communicator_Mutex;
	boost::shared_mutex *NS3_Communicator_Mutex_Send;
	boost::shared_mutex *NS3_Communicator_Mutex_Receive;
	std::vector<boost::shared_mutex*> mutex_collection;
	explicit NS3_Communicator(const MutexStrategy& mtxStrat, int id=-1);
	Entity::UpdateStatus update(timeslice now);
	void load(const std::map<std::string, std::string>& configProps);
	bool frame_init(timeslice now);
	Entity::UpdateStatus frame_tick(timeslice now);
	void frame_output(timeslice now);

	void enable() { enabled = true; }
	void disable() { enabled = false; }
	bool isEnabled() { return enabled; }

	sim_mob::DataContainer &getSendBuffer();
	sim_mob::DataContainer &getReceiveBuffer();
	void popReceiveBuffer(DATA_MSG_PTR & value);
	// I think agents dont need to use addSendBuffer() as long as
	//they set their outgoing buffer to point to communicator's sendbuffer
	//still addSendBuffer() will be used for control messages sent by the communicator itself
	void addSendBuffer(sim_mob::DATA_MSG_PTR&);
	void addSendBuffer(sim_mob::DataContainer &value);
	void addSendBuffer(std::vector<DATA_MSG_PTR> &value);

	std::vector<boost::shared_mutex *> subscribeEntity(sim_mob::CommunicationSupport&);
	bool unSubscribeEntity(sim_mob::CommunicationSupport&);
	bool unSubscribeEntity(const sim_mob::Entity * agent);
	static NS3_Communicator& GetInstance() { return NS3_Communicator::instance; }
};
//at present, we are not supposed to have more than one communicator(it is a singleton) still we put this for workgroups

}

//#include "entities/Agent.hpp"
//namespace sim_mob {
//class Communicator : public Agent
//{
//public:
//	Communicator(const MutexStrategy& mtxStrat, int id=-1)
//	: Agent(mtxStrat, id)
//	{
//
//	}
//	Entity::UpdateStatus update(timeslice now)
//	{
//		ConfigParams & config = ConfigParams::GetInstance();
//		timeval loop_start_time = ConfigParams::GetInstance().realSimStartTime;
//		std::ofstream simTimeStream;
//		simTimeStream.open("/home/vahid/simmobility/dev/Basic/Sim_Mob_Time");
//		std::ostringstream simTimeStringStream;
//		//write time into a file
//		simTimeStringStream.str("");
//		__time_t carryOverSeconds = (loop_start_time.tv_usec + now.ms()*1000) / 1000000;
//		__useconds_t uSec = (loop_start_time.tv_usec + now.ms()*1000) % 1000000;
//		timeval t;
//		t.tv_sec = loop_start_time.tv_sec + carryOverSeconds;
//		t.tv_usec = uSec; //seconds already carried over, so no need of adding it with loop_start_time.tv_usec
//		simTimeStringStream << t.tv_sec << " " << t.tv_usec ;
//		simTimeStream.seekp(0);
//		simTimeStream << simTimeStringStream.str() << std::endl;//endl will reflect the buffer to the file, hence necessary
//		////////////////////////
//
//	}
//	void load(const std::map<std::string, std::string>& configProps)
//	{
//
//	}
//	bool frame_init(timeslice now)
//	{
//
//	}
//	sim_mob::Entity::UpdateStatus frame_tick(timeslice)
//	{
//
//	}
//	void frame_output(timeslice)
//	{
//
//	}
//};
//
//}
