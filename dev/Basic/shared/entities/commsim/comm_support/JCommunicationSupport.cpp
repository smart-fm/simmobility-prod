#include "JCommunicationSupport.hpp"
#include "entities/commsim/communicator/broker/Broker.hpp"


//using namespace sim_mob;
namespace sim_mob
{
template<class T>
JCommunicationSupport<T>::JCommunicationSupport(sim_mob::Broker& managingBroker, sim_mob::Agent& entity_)
	:	/*CommSupp_Mutex(new boost::shared_mutex),*/
	 	entity(entity_),
		communicator(managingBroker),
	 	outgoing(managingBroker.getSendBuffer()),
		incomingIsDirty(false),
		outgoingIsDirty(false),
		writeIncomingDone(false),
		readOutgoingDone(false),
		agentUpdateDone(false),
		cnt_1(0), cnt_2(0)
	{
		subscribed = false;
		subscriptionCallback = &JCommunicationSupport::setSubscribed;
	}


template<class T>
void JCommunicationSupport<T>::setSubscribed(bool value)
{
	subscribed = value;
}
template<class T>
void JCommunicationSupport<T>::setMutexes(std::vector<boost::shared_ptr<boost::shared_mutex> > &value)
{
	Broker_Mutexes = value;
}

	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
template<class T>
	BufferContainer<T>& JCommunicationSupport<T>::getIncoming() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incoming;
	}
	//give a copy for temporary use and then clears. all done in one uninterrupted lock
template<class T>
	void JCommunicationSupport<T>::getAndClearIncoming(BufferContainer<T> &values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		values = incoming;
		incoming.clear();
	}


template<class T>
BufferContainer<T>& JCommunicationSupport<T>::getOutgoing() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return outgoing;
	}
template<class T>
	void JCommunicationSupport<T>::setIncoming(BufferContainer<T> values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		incoming = values;
	}
template<class T>
	bool JCommunicationSupport<T>::popIncoming(T &var)
	{
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incoming.pop(var);
	}
//	void JCommunicationSupport::setOutgoing(BufferContainer values) {
//		//boost::unique_lock< boost::shared_mutex > lock(*CommSupp_Mutex);
//		outgoing = values;
//		outgoingIsDirty = true;
//	}


template<class T>
void JCommunicationSupport<T>::addIncoming(T value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		std::cout << "addIncoming_Acquiring_receive_lock_DONE" << std::endl;
		incoming.add(value);
		incomingIsDirty = true;
	}
template<class T>
	void JCommunicationSupport<T>::addOutgoing(T value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		std::cout << "outgoingsize-before[" << outgoing.get().size() << "]" << std::endl;
		outgoing.add(value);
		std::cout << "outgoingsize-after[" << outgoing.get().size() << "]" << std::endl;
		outgoingIsDirty = true;
	}

template<class T>
	void JCommunicationSupport<T>::setwriteIncomingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		writeIncomingDone = value;
	}
template<class T>
	void JCommunicationSupport<T>::setWriteOutgoingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		readOutgoingDone = value;
	}
template<class T>
	void JCommunicationSupport<T>::setAgentUpdateDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[0]));
		agentUpdateDone = value;
	}
template<class T>
	bool &JCommunicationSupport<T>::iswriteIncomingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return writeIncomingDone;
	}
template<class T>
	bool &JCommunicationSupport<T>::isreadOutgoingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return readOutgoingDone;
	}
template<class T>
	bool &JCommunicationSupport<T>::isAgentUpdateDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[0]));
			return agentUpdateDone;
	}

template<class T>
	bool &JCommunicationSupport<T>::isOutgoingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[1]));
		return outgoingIsDirty;
	}
template<class T>
	bool &JCommunicationSupport<T>::isIncomingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Broker_Mutexes[2]));
		return incomingIsDirty;
	}

//todo
template<class T>
	void JCommunicationSupport<T>::reset(){
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
template<class T>
	void JCommunicationSupport<T>::init(){

	}

	//this is used to subscribe the drived class
	//(which is also an agent) to the communicator agent
	template<class T>
	bool JCommunicationSupport<T>::subscribe(sim_mob::Agent* subscriber, sim_mob::Broker &communicator)
	{
//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(subscriber);

		return communicator.subscribeEntity(this);
//		std::cout << "agent[" << &getEntity() << "] was subscribed with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	template<class T>
	const sim_mob::Agent& JCommunicationSupport<T>::getEntity()
	{
		return entity;
	}

};
