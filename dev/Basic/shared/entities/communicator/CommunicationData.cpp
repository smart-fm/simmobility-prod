#include "CommunicationData.hpp"
#include<cstdio>
namespace sim_mob
{
///////////////////////////////////
//template<class Archive>
//void DataContainer::serialize(Archive &ar, const unsigned int version) {
//	DATA_MSG_PTR value;
//	ar.register_type(static_cast<dataMessage *>(NULL));
////    	BOOST_FOREACH(value, buffer)
////    	//takes ar to your class and gives him the information about your class
////    		value->registerType(ar);
//	ar & buffer;
//}
DataContainer::DataContainer():myLock(new Lock){}

void DataContainer::add(DATA_MSG_PTR value) {
	WriteLock(*myLock);
	buffer.push_back(value);
}

void DataContainer::add(std::vector<DATA_MSG_PTR> values) {
	WriteLock(*myLock);
	buffer.insert(buffer.end(), values.begin(), values.end());
}

void DataContainer::add(DataContainer & value) {
	WriteLock(*myLock);
	add(value.get());
}

void DataContainer::reset() {
	WriteLock(*myLock);
	DATA_MSG_PTR value;
	std::cout << "resetting a buffer of size " << buffer.size() << std::endl;

	BOOST_FOREACH(value, buffer)
		delete value;
	std::cout << "resetting " << std::endl;
	buffer.clear();
	std::cout << " done" << std::endl;
}

std::vector<DATA_MSG_PTR>& DataContainer::get() {
	return buffer;
}

bool DataContainer::pop(DATA_MSG_PTR & var) {
	WriteLock(*myLock);
	if (buffer.size() < 1)
		return false;
	var = buffer.front();
	buffer.erase(buffer.begin());
}

bool DataContainer::empty(){
	return buffer.empty();
}
/////////////////////////////////////
//subscriptionInfo::subscriptionInfo(
//		sim_mob::Entity *  agent_ ,
//		bool& incomingIsDirty_ ,
//		bool& outgoingIsDirty_ ,
//		bool& writeIncomingDone_,
//		bool& readOutgoingDone_ ,
//		bool& agentUpdateDone_,
//		DataContainer& incoming_,
//		DataContainer& outgoing_
//
//		)
//:
//	agent(agent_),
//	incomingIsDirty(incomingIsDirty_),
//	outgoingIsDirty(outgoingIsDirty_),
//	writeIncomingDone(writeIncomingDone_),
//	readOutgoingDone(readOutgoingDone_),
//	agentUpdateDone(agentUpdateDone_),
//	incoming(incoming_),
//	outgoing(outgoing_)
//{
//	cnt_1 = cnt_2 = 0;
//	myLock = boost::shared_ptr<Lock>(new Lock);//will be deleted itself :)
//}
//
////subscriptionInfo & subscriptionInfo::operator=(const subscriptionInfo&) {
////	return *this;
////}
//
//void subscriptionInfo::setEntity(sim_mob::Entity* value)
//{
//	agent = value;
//}
//sim_mob::Entity* subscriptionInfo::getEntity()
//{
//	return agent;
//}
////we use original dataMessage(or DATA_MSG) type to avoid wrong read/write
//DataContainer& subscriptionInfo::getIncoming() {
//	ReadLock Lock(*myLock);
//	return incoming;
//}
//DataContainer& subscriptionInfo::getOutgoing() {
//	ReadLock Lock(*myLock);
//	return outgoing;
//}
//void subscriptionInfo::setIncoming(DataContainer values) {
//	WriteLock(*myLock);
//	incoming = values;
//	incomingIsDirty = true;
//}
//void subscriptionInfo::setOutgoing(DataContainer values) {
//	WriteLock(*myLock);
//	outgoing = values;
//	outgoingIsDirty = true;}
//
//void subscriptionInfo::addIncoming(DATA_MSG_PTR value) {
//	WriteLock(*myLock);
//	incoming.add(value);
//	incomingIsDirty = true;
//}
//void subscriptionInfo::addOutgoing(DATA_MSG_PTR value) {
//std::cout << this << " : subscriptionInfo::addOutgoing=>pushing data to " << &outgoing << std::endl;
//WriteLock(*myLock);
//outgoing.add(value);
//outgoingIsDirty = true;
//}
//
//void subscriptionInfo::setwriteIncomingDone(bool value) {
//	WriteLock(*myLock);
//	writeIncomingDone = value;
//}
//void subscriptionInfo::setWriteOutgoingDone(bool value) {
//	WriteLock(*myLock);
//	readOutgoingDone = value;
//}
//void subscriptionInfo::setAgentUpdateDone(bool value) {
//	WriteLock(*myLock);
//	agentUpdateDone = value;
//}
//bool subscriptionInfo::iswriteIncomingDone() {
//	ReadLock Lock(*myLock);
//	return writeIncomingDone;
//}
//bool subscriptionInfo::isreadOutgoingDone() {
//	ReadLock Lock(*myLock);
//	return readOutgoingDone;
//}
//bool subscriptionInfo::isAgentUpdateDone() {
//	ReadLock Lock(*myLock);
//	return agentUpdateDone;
//}
//
//bool subscriptionInfo::isOutgoingDirty() {
//	ReadLock Lock(*myLock);
//	return outgoingIsDirty;
//}
//bool subscriptionInfo::isIncomingDirty() {
//	ReadLock Lock(*myLock);
//	return incomingIsDirty;
//}
//
//void subscriptionInfo::reset()
//{
//	WriteLock Lock(*myLock);
//	incomingIsDirty = false ;
//	outgoingIsDirty = false ;
//	agentUpdateDone = false ;
//	readOutgoingDone = false ;
//	writeIncomingDone = false ;
////	incoming.reset();
////	outgoing.reset();
//	cnt_1 = cnt_2 = 0;
//}
}//namespace sim_mob
