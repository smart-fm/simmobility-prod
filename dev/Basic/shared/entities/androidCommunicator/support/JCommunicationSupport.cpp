#include "JCommunicationSupport.hpp"
#include "../communicator/broker/Broker.hpp"


using namespace sim_mob;
namespace sim_mob
{
	JCommunicationSupport::JCommunicationSupport(sim_mob::Agent& entity_)
	:	/*CommSupp_Mutex(new boost::shared_mutex),*/
	 	entity(entity_),
		communicator(sim_mob::Broker::GetInstance()),
	 	outgoing(sim_mob::Broker::GetInstance().getSendBuffer()),
		incomingIsDirty(false),
		outgoingIsDirty(false),
		writeIncomingDone(false),
		readOutgoingDone(false),
		agentUpdateDone(false)
	{
		subscribed = false;
		subscriptionCallback = &JCommunicationSupport::setSubscribed;
	}

void JCommunicationSupport::setSubscribed(bool value)
{
	subscribed = value;
}
void JCommunicationSupport::setMutexes(std::vector<boost::shared_ptr<boost::shared_mutex> > &value)
{
	Broker_Mutexes = value;
}

	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	BufferContainer& JCommunicationSupport::getIncoming() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incoming;
	}
	//give a copy for temporary use and then clears. all done in one uninterrupted lock
	void JCommunicationSupport::getAndClearIncoming(BufferContainer &values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		values = incoming;
		incoming.clear();
	}

	BufferContainer& JCommunicationSupport::getOutgoing() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return outgoing;
	}
	void JCommunicationSupport::setIncoming(BufferContainer values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		incoming = values;
	}
	bool JCommunicationSupport::popIncoming(DataElement &var)
	{
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incoming.pop(var);
	}
//	void JCommunicationSupport::setOutgoing(BufferContainer values) {
//		//boost::unique_lock< boost::shared_mutex > lock(*CommSupp_Mutex);
//		outgoing = values;
//		outgoingIsDirty = true;
//	}

	void JCommunicationSupport::addIncoming(DataElement value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		std::cout << "addIncoming_Acquiring_receive_lock_DONE" << std::endl;
		incoming.add(value);
		incomingIsDirty = true;
	}
	void JCommunicationSupport::addOutgoing(DataElement value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		std::cout << "outgoingsize-before[" << outgoing.get().size() << "]" << std::endl;
		outgoing.add(value);
		std::cout << "outgoingsize-after[" << outgoing.get().size() << "]" << std::endl;
		outgoingIsDirty = true;
	}

	void JCommunicationSupport::setwriteIncomingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		writeIncomingDone = value;
	}
	void JCommunicationSupport::setWriteOutgoingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		readOutgoingDone = value;
	}
	void JCommunicationSupport::setAgentUpdateDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[0]));
		agentUpdateDone = value;
	}
	bool &JCommunicationSupport::iswriteIncomingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return writeIncomingDone;
	}
	bool &JCommunicationSupport::isreadOutgoingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return readOutgoingDone;
	}
	bool &JCommunicationSupport::isAgentUpdateDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[0]));
			return agentUpdateDone;
	}

	bool &JCommunicationSupport::isOutgoingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return outgoingIsDirty;
	}
	bool &JCommunicationSupport::isIncomingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incomingIsDirty;
	}

//todo
	void JCommunicationSupport::reset(){
		{
			boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutexes[0]);
			agentUpdateDone = false ;
			cnt_1 = cnt_2 = 0;
		}
		{
			boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutexes[1]);
			outgoingIsDirty = false ;
			readOutgoingDone = false ;
		}
		{
			boost::unique_lock< boost::shared_mutex > lock(*Broker_Mutexes[2]);
			incomingIsDirty = false ;
			writeIncomingDone = false ;
		}
	}
	void JCommunicationSupport::init(){

	}
	//this is used to subscribe the drived class
	//(which is also an agent) to the communicator agent
	bool JCommunicationSupport::subscribe(sim_mob::Agent* subscriber, sim_mob::Broker &communicator = sim_mob::Broker::GetInstance())
	{
//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(subscriber);

		return communicator.subscribeEntity(*this);
//		std::cout << "agent[" << &getEntity() << "] was subscribed with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	const sim_mob::Agent& JCommunicationSupport::getEntity()
	{
		return entity;
	}

};
