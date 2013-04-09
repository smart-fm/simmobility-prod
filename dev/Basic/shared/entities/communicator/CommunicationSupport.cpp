#include "CommunicationSupport.hpp"
#include "Communicator.hpp"


using namespace sim_mob;
namespace sim_mob
{
	CommunicationSupport::CommunicationSupport(sim_mob::Entity& entity_)
	:	myLock(new Lock),
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
		ReadLock Lock(*myLock);
		return incoming;
	}
	DataContainer& CommunicationSupport::getOutgoing() {
		ReadLock Lock(*myLock);
		return outgoing;
	}
	void CommunicationSupport::setIncoming(DataContainer values) {
		WriteLock(*myLock);
		incoming = values;
	}
	bool CommunicationSupport::popIncoming(DATA_MSG_PTR &var)
	{
		WriteLock(*myLock);
		return incoming.pop(var);
	}
//	void CommunicationSupport::setOutgoing(DataContainer values) {
//		WriteLock(*myLock);
//		outgoing = values;
//		outgoingIsDirty = true;
//	}

	void CommunicationSupport::addIncoming(DATA_MSG_PTR value) {
		WriteLock(*myLock);
		incoming.add(value);
	}
	void CommunicationSupport::addOutgoing(DATA_MSG_PTR value) {
		std::cout << this << " : CommunicationSupport::addOutgoing=>pushing data[" << value << "] to outgoing[" << &outgoing << "]" <<  std::endl;
	WriteLock(*myLock);
	outgoing.add(value);
	std::cout << "push done "  <<  std::endl;
	outgoingIsDirty = true;
	}

	void CommunicationSupport::setwriteIncomingDone(bool value) {
		WriteLock(*myLock);
		writeIncomingDone = value;
	}
	void CommunicationSupport::setWriteOutgoingDone(bool value) {
		WriteLock(*myLock);
		readOutgoingDone = value;
	}
	void CommunicationSupport::setAgentUpdateDone(bool value) {
		WriteLock(*myLock);
		agentUpdateDone = value;
	}
	bool &CommunicationSupport::iswriteIncomingDone() {
		ReadLock Lock(*myLock);
		return writeIncomingDone;
	}
	bool &CommunicationSupport::isreadOutgoingDone() {
		ReadLock Lock(*myLock);
		return readOutgoingDone;
	}
	bool &CommunicationSupport::isAgentUpdateDone() {
		ReadLock Lock(*myLock);
		return agentUpdateDone;
	}

	bool &CommunicationSupport::isOutgoingDirty() {
		ReadLock Lock(*myLock);
		return outgoingIsDirty;
	}
	bool &CommunicationSupport::isIncomingDirty() {
		ReadLock Lock(*myLock);
		return incomingIsDirty;
	}

//todo
	void CommunicationSupport::reset(){
		WriteLock Lock(*myLock);
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

		communicator.subscribeEntity(*this);
		std::cout << "agent[" << &getEntity() << "] was subscribed with outgoing[" << &(getOutgoing()) << "]" << std::endl;
	}
	const sim_mob::Entity& CommunicationSupport::getEntity()
	{
		return entity;
	}

};
