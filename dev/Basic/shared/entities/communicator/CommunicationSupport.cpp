#include "CommunicationSupport.hpp"
#include "NS3/NS3_Communicator/NS3_Communicator.hpp"


using namespace sim_mob;
namespace sim_mob
{

	CommunicationSupport::CommunicationSupport(sim_mob::Agent& entity_)
	:	/*CommSupp_Mutex(new boost::shared_mutex),*/
	 	entity(entity_),
		communicator(sim_mob::NS3_Communicator::GetInstance()),
	 	outgoing(sim_mob::NS3_Communicator::GetInstance().getSendBuffer()),
		incomingIsDirty(false),
		outgoingIsDirty(false),
		writeIncomingDone(false),
		readOutgoingDone(false),
		agentUpdateDone(false)
	{
		subscribed = false;
		subscriptionCallback = &CommunicationSupport::setSubscribed;
	}
//	subscriptionInfo CommunicationSupport::getSubscriptionInfo(){
//		subscriptionInfo info(
//				(sim_mob::Entity*)0,
//				isIncomingDirty(),
//				isOutgoingDirty(),
//				iswriteIncomingDone(),
//				isreadOutgoingDone(),
//				isAgentUpdateDone(),
//				getIncoming(),
//				getOutgoing()
//				);
//		return info;
//	}
void CommunicationSupport::setSubscribed(bool value)
{
	subscribed = value;
}

	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	DataContainer& CommunicationSupport::getIncoming() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		return incoming;
	}
	//give a copy for temporary use and then clears. all done in one uninterrupted lock
	void CommunicationSupport::getAndClearIncoming(DataContainer &values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		values = incoming;
		incoming.clear();
	}

	DataContainer& CommunicationSupport::getOutgoing() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
		return outgoing;
	}
	void CommunicationSupport::setIncoming(DataContainer values) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		incoming = values;
	}
	bool CommunicationSupport::popIncoming(DATA_MSG_PTR &var)
	{
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		return incoming.pop(var);
	}
//	void CommunicationSupport::setOutgoing(DataContainer values) {
//		//boost::unique_lock< boost::shared_mutex > lock(*CommSupp_Mutex);
//		outgoing = values;
//		outgoingIsDirty = true;
//	}

	void CommunicationSupport::addIncoming(DATA_MSG_PTR value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		std::cout << "addIncoming_Acquiring_receive_lock_DONE" << std::endl;
		incoming.add(value);
		incomingIsDirty = true;
	}
	void CommunicationSupport::addOutgoing(DATA_MSG_PTR value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
		std::cout << "outgoingsize-before[" << outgoing.get().size() << "]" << std::endl;
		outgoing.add(value);
		std::cout << "outgoingsize-after[" << outgoing.get().size() << "]" << std::endl;
		outgoingIsDirty = true;
	}

	void CommunicationSupport::setwriteIncomingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		writeIncomingDone = value;
	}
	void CommunicationSupport::setWriteOutgoingDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
		readOutgoingDone = value;
	}
	void CommunicationSupport::setAgentUpdateDone(bool value) {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[0]));
		agentUpdateDone = value;
	}
	bool &CommunicationSupport::iswriteIncomingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		return writeIncomingDone;
	}
	bool &CommunicationSupport::isreadOutgoingDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
		return readOutgoingDone;
	}
	bool &CommunicationSupport::isAgentUpdateDone() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[0]));
			return agentUpdateDone;
	}

	bool &CommunicationSupport::isOutgoingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
		return outgoingIsDirty;
	}
	bool &CommunicationSupport::isIncomingDirty() {
		boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[2]));
		return incomingIsDirty;
	}

//todo
	void CommunicationSupport::reset(){
		{
			boost::unique_lock< boost::shared_mutex > lock(*Communicator_Mutexes[0]);
			agentUpdateDone = false ;
			cnt_1 = cnt_2 = 0;
		}
		{
			boost::unique_lock< boost::shared_mutex > lock(*Communicator_Mutexes[1]);
			outgoingIsDirty = false ;
			readOutgoingDone = false ;
		}
		{
			boost::unique_lock< boost::shared_mutex > lock(*Communicator_Mutexes[2]);
			incomingIsDirty = false ;
			writeIncomingDone = false ;
		}
	}
	void CommunicationSupport::init(){

	}
	//this is used to subscribe the drived class
	//(which is also an agent) to the communicator agent
	bool CommunicationSupport::subscribe(sim_mob::Entity* subscriber, sim_mob::NS3_Communicator &communicator = sim_mob::NS3_Communicator::GetInstance())
	{
//		//todo here you are copying twice while once is possibl, I guess.
//		subscriptionInfo info = getSubscriptionInfo();
//		info.setEntity(subscriber);

		Communicator_Mutexes = communicator.subscribeEntity(*this);
//		std::cout << "agent[" << &getEntity() << "] was subscribed with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	const sim_mob::Agent& CommunicationSupport::getEntity()
	{
		return entity;
	}
}//namespace sim_mob;
