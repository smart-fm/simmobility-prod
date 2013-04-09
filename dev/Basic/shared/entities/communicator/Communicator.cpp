#include "Communicator.hpp"
#include "CommunicationSupport.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include <boost/thread/mutex.hpp>

#include <boost/thread.hpp>

///
namespace sim_mob {

//sim_mob::All_NS3_Communicators sim_mob::NS3_Communicator::all_NS3_cummunication_agents;
NS3_Communicator::NS3_Communicator(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id), commImpl(&sendBuffer, &receiveBuffer)
{
//	std::cout << " Communicator's SendBuffer address [" << &sendBuffer <<  ":" << &(sendBuffer.buffer) << "]" << std::endl;
}
//////////////////////////////////////////
// Simple singleton implementation
//////////////////////////////////////////
sim_mob::NS3_Communicator sim_mob::NS3_Communicator::instance(MtxStrat_Locked, 0);


/*
 * Algorithm: is like this:
 * 1- Read all outgoing messages from agents and process them(send the messages to NS3)
 * 2- Read all incoming messages (sent from NS3 and write them to hand them to agents
 * 3- clear the flags
 */
Entity::UpdateStatus NS3_Communicator::update(timeslice now)
{
	//	commImpl.shortCircuit();
	std::cout << "communicator tick:"<< now.frame() << " ================================================\n";
	boost::thread outGoing_thread(boost::bind(&sim_mob::NS3_Communicator::processOutgoingData,this,now));
	boost::thread inComing_thread(boost::bind(&sim_mob::NS3_Communicator::processIncomingData,this,now));
	outGoing_thread.join();
	inComing_thread.join();
	reset();
	std::cout <<"------------------------------------------------------\n";
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}
void NS3_Communicator::printSubscriptionList(timeslice now)
{
	std::ostringstream out("");
	out << "printSubscriptionList\n" ;
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		out.str("");
		out << "communicator tick:" << now.frame() << ": [" << it->first <<"]";
		out << "[" << (it->second.isAgentUpdateDone()? "UPDO" : "UPND") <<  ":" << (it->second.isOutgoingDirty()? "DRT" : "NDT") << "] out[" << &(it->second.getOutgoing()) << " : size " << it->second.getOutgoing().get().size() << "]";
		//outgoing
		DATA_MSG_PTR out_it;
		BOOST_FOREACH(out_it, it->second.getOutgoing().get())
			out << " " << out_it->str ;

		std::cout << out.str() << std::endl;
	}
}
bool NS3_Communicator::deadEntityCheck(sim_mob::CommunicationSupport & info) {
	//some top notch optimizasion! to check if the agent is alive at all?
	info.cnt_1++;
	if (info.cnt_1 < 1000)
		return false;

	//you or your worker are probably dead already. you just don't know it
	if (!(info.getEntity().currWorker))
		return true;
	//one more check to see if the entity is deleted
	const std::vector<sim_mob::Entity*> & managedEntities_ = info.getEntity().currWorker->getEntities();
	std::vector<sim_mob::Entity*>::const_iterator it = managedEntities_.begin();
	for (std::vector<sim_mob::Entity*>::const_iterator it =	managedEntities_.begin(); it != managedEntities_.end(); it++)
	{
		//agent is still being managed, so it is not dead
		if (*it == &(info.getEntity()))
			return false;
	}

	return true;
}

//iterate the entire subscription list looking for
//those who are not done with their update and check if they are dead.
//you better hope they are dead otherwise you have to hold the simulation
//tick waiting for them to finish
void NS3_Communicator::refineSubscriptionList() {
	WriteLock(*myLock);
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it, it_end(subscriptionList.end());
	for(it = subscriptionList.begin(); it != it_end; it++)
	{
		const sim_mob::Entity * target = (*it).first;
		std::cout<< "rfn checking agent [" << target << "]" << std::endl;
		if(duplicateEntityDoneChecker.find(target) == duplicateEntityDoneChecker.end() ) {
			std::cout << "aget[" << target << "] is already Done" << std::endl;
			continue;
		}
		//you or your worker are probably dead already. you just don't know it
		if (!target->currWorker)
			{
//				boost::upgrade_to_unique_lock< Lock > ulock(lock);
				subscriptionList.erase(target);
				std::cout << "worker " << target->currWorker << " is dead so Agent " << target << " is removed from the list" << std::endl;
				continue;
			}
		const std::vector<sim_mob::Entity*> & managedEntities_ = (target->currWorker)->getEntities();
		std::vector<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
		if(it_entity == managedEntities_.end())
		{
//			boost::upgrade_to_unique_lock< Lock > ulock(lock);
			std::cout << "_Agent " << target << " is removed from the list" << std::endl;
			subscriptionList.erase(target);
			continue;
		}
		else
		{
			std::cout << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
		}
	}
}

void NS3_Communicator::bufferSend()
{
	WriteLock(*myLock);
	std::cout << "Sending the buffer\n";
	commImpl.send(&sendBuffer);
	sendBuffer.reset();
}
bool NS3_Communicator::allAgentUpdatesDone()
{
	ReadLock(*myLock);
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::const_iterator it, it_end(subscriptionList.end());

	for(it = subscriptionList.begin(); it != it_end; it++)
	{
		sim_mob::CommunicationSupport & info = subscriptionList.at(it->first);//easy read
		if (info.isAgentUpdateDone())
		{
			duplicateEntityDoneChecker.insert(it->first);
		}
	}
	std::cout << "subscriptionList size = " << subscriptionList.size() << std::endl;
	std::cout << "duplicateEntityDoneChecker size = " << duplicateEntityDoneChecker.size() << std::endl;
	return(duplicateEntityDoneChecker.size() >= subscriptionList.size());
}

//todo: think of a better return value than just void
bool NS3_Communicator::processOutgoingData(timeslice now)
{
	do
	{

		boost::thread thread_rifineSubscriptionList(boost::bind(&NS3_Communicator::refineSubscriptionList,this));
		if(!sendBuffer.empty())
		{
			boost::thread thread_send(boost::bind(&NS3_Communicator::bufferSend,this));
			thread_send.join();
			std::cout << now.frame() << " Thread_send Joined\n";
		}
		thread_rifineSubscriptionList.join();
		std::cout << now.frame() << "Thread_rifineSubscriptionList Joined\n";
		printSubscriptionList(now);
	}while(!allAgentUpdatesDone());
}

//todo: think of a better return value than just void
void NS3_Communicator::processIncomingData(timeslice now)
{
//	if(now.frame() != 4)
//	{
//		return;
//		std::cout << std::endl;
//	}
  DataContainer *receiveBatch; //will contain data belonging to many agents
  const commResult &res = commImpl.receive(receiveBatch);
  if(res.getResult() != commResult::success) return;
  DATA_MSG_PTR it;
  //distribute among agents
  BOOST_FOREACH(it,receiveBatch->get())
  {
	  //get the reference to the receiving agent's subscription record
	  sim_mob::Entity * receiver = (sim_mob::Entity *) (it->receiver);
	  sim_mob::CommunicationSupport & info = subscriptionList.at(receiver);
	  //the receiving agent's subscription record has a reference to its incoming buffer
	  info.addIncoming(it);
  }

}

void NS3_Communicator::reset()
{
	WriteLock(*myLock);
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		sim_mob::CommunicationSupport &info = it->second;
		info.reset();

	}
	sendBuffer.reset();
	receiveBuffer.reset();

}
void NS3_Communicator::reset(sim_mob::CommunicationSupport &info)
{
	info.reset();
}

void  NS3_Communicator::subscribeEntity(sim_mob::CommunicationSupport & value)
{
	subscriptionList.insert(std::pair<const sim_mob::Entity*, sim_mob::CommunicationSupport &>(&(value.getEntity()),value));
	std::cout << "Subscribed agent[" << &value.getEntity() << "] with outgoing[" << &(subscriptionList.at(&value.getEntity()).getOutgoing()) << ":" << &sendBuffer << "]" << std::endl;
}
bool  NS3_Communicator::unSubscribeEntity(sim_mob::CommunicationSupport &value)
{
	return subscriptionList.erase(&value.getEntity());
}

bool  NS3_Communicator::unSubscribeEntity(const sim_mob::Entity * agent)
{
	return subscriptionList.erase(agent);
}

sim_mob::DataContainer &NS3_Communicator::getSendBuffer()
{
//	std::cout << "NS3_Communicator::getSendBuffer => sending buffer[" << &sendBuffer << "]" << std::endl;
	return sendBuffer;
}

void NS3_Communicator::popReceiveBuffer(DATA_MSG_PTR & value)
{
	sim_mob::WriteLock Lock(*myLock);
	value = receiveBuffer.get().front();
	receiveBuffer.get().erase(receiveBuffer.get().begin());
}

sim_mob::DataContainer &NS3_Communicator::getReceiveBuffer()
{
	sim_mob::ReadLock Lock(*myLock);
	return receiveBuffer;
}
//void NS3_Communicator::addSendBuffer(sim_mob::DATA_MSG_PTR &value){
//	sim_mob::WriteLock Lock(*myLock);
//	sendBuffer.add(value);
//}
void NS3_Communicator::addSendBuffer(sim_mob::DataContainer &value){
	sim_mob::WriteLock Lock(*myLock);
	sendBuffer.add(value);
}
void NS3_Communicator::addSendBuffer(std::vector<DATA_MSG_PTR> &value){
	sim_mob::WriteLock Lock(*myLock);
	sendBuffer.add(value);
}

void NS3_Communicator::load(const std::map<std::string, std::string>& configProps)
{

}

bool NS3_Communicator::frame_init(timeslice now)
{

}

Entity::UpdateStatus NS3_Communicator::frame_tick(timeslice now)
{

}
void NS3_Communicator::frame_output(timeslice now)
{

}

}//namespace
