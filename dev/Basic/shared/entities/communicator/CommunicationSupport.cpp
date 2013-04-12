#include "CommunicationSupport.hpp"
#include "Communicator.hpp"


using namespace sim_mob;
namespace sim_mob
{
	CommunicationSupport::CommunicationSupport(sim_mob::Entity& entity_)
	:	CommSupp_Mutex(new Lock),
	 	entity(entity_),
		communicator(sim_mob::NS3_Communicator::GetInstance()),
	 	outgoing(sim_mob::NS3_Communicator::GetInstance().getSendBuffer()),
		incomingIsDirty(false),
		outgoingIsDirty(false),
		writeIncomingDone(false),
		readOutgoingDone(false),
		agentUpdateDone(false)
	{

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

	//we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
	DataContainer& CommunicationSupport::getIncoming() {
//		WriteLock Lock(*Communicator_Mutex);
		return incoming;
	}
	DataContainer& CommunicationSupport::getOutgoing() {
//		WriteLock Lock(*Communicator_Mutex);
		return outgoing;
	}
	void CommunicationSupport::setIncoming(DataContainer values) {
		WriteLock Lock(*Communicator_Mutex);
		incoming = values;
	}
	bool CommunicationSupport::popIncoming(DATA_MSG_PTR &var)
	{
		WriteLock Lock(*Communicator_Mutex);
		return incoming.pop(var);
	}
//	void CommunicationSupport::setOutgoing(DataContainer values) {
//		//WriteLock(*CommSupp_Mutex);
//		outgoing = values;
//		outgoingIsDirty = true;
//	}

	void CommunicationSupport::addIncoming(DATA_MSG_PTR value) {
		WriteLock Lock(*Communicator_Mutex);
		std::cout << this << " : CommunicationSupport::addIncoming=>storing incoming data" <<  std::endl;
		incoming.add(value);
		incomingIsDirty = true;
	}
	void CommunicationSupport::addOutgoing(DATA_MSG_PTR value) {
		WriteLock Lock(*Communicator_Mutex);
		std::cout << this << " : CommunicationSupport::addOutgoing=>pushing data[" << value << "]" <<  std::endl;
	outgoing.add(value);
//	std::cout << "push done "  <<  std::endl;
	outgoingIsDirty = true;
	}

	void CommunicationSupport::setwriteIncomingDone(bool value) {
		WriteLock Lock(*Communicator_Mutex);
		writeIncomingDone = value;
	}
	void CommunicationSupport::setWriteOutgoingDone(bool value) {
		WriteLock Lock(*Communicator_Mutex);
		readOutgoingDone = value;
	}
	void CommunicationSupport::setAgentUpdateDone(bool value) {
		WriteLock Lock(*Communicator_Mutex);
		agentUpdateDone = value;
	}
	bool &CommunicationSupport::iswriteIncomingDone() {
		WriteLock Lock(*Communicator_Mutex);
		return writeIncomingDone;
	}
	bool &CommunicationSupport::isreadOutgoingDone() {
		WriteLock Lock(*Communicator_Mutex);
		return readOutgoingDone;
	}
	bool &CommunicationSupport::isAgentUpdateDone() {
		WriteLock Lock(*Communicator_Mutex);
			return agentUpdateDone;
	}

	bool &CommunicationSupport::isOutgoingDirty() {
		WriteLock Lock(*Communicator_Mutex);
		return outgoingIsDirty;
	}
	bool &CommunicationSupport::isIncomingDirty() {
		WriteLock Lock(*Communicator_Mutex);
		return incomingIsDirty;
	}

//todo
	void CommunicationSupport::reset(){
		WriteLock Lock(*Communicator_Mutex);
		outgoingIsDirty = false ;
		incomingIsDirty = false ;
		agentUpdateDone = false ;
		writeIncomingDone = false ;
		readOutgoingDone = false ;
		cnt_1 = cnt_2 = 0;
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

		Communicator_Mutex = communicator.subscribeEntity(*this);
		std::cout << "agent[" << &getEntity() << "] was subscribed with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	const sim_mob::Entity& CommunicationSupport::getEntity()
	{
		return entity;
	}

};
