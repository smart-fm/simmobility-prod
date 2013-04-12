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
	NS3_Communicator_Mutex.reset(new Lock);
	myLocalLock.reset(new Lock);
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
////	printSubscriptionList(now);
//	boost::thread outGoing_thread(boost::bind(&sim_mob::NS3_Communicator::processOutgoingData,this,now));
//	boost::thread inComing_thread(boost::bind(&sim_mob::NS3_Communicator::processIncomingData,this,now));
//	outGoing_thread.join();
//	inComing_thread.join();
	processOutgoingData(now);
	processIncomingData(now);
//	reset();
	std::cout <<"------------------------------------------------------\n";
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}
void NS3_Communicator::printSubscriptionList(timeslice now)
{
	WriteLock(*NS3_Communicator_Mutex);
	std::ostringstream out("");
	out << "printSubscriptionList\n" ;
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		out.str("");
		out << "communicator tick:" << now.frame() << ": [" << it->first <<"]";
		out << "[" << (it->second.isAgentUpdateDone()? "UPDO" : "UPND") <<  "]\n" ;
		//outgoing
		out << "[" << (it->second.isOutgoingDirty()? "ODT" : "OND") << "] size[" << it->second.getOutgoing().get().size() << "]";
		DATA_MSG_PTR out_it;
		BOOST_FOREACH(out_it, it->second.getOutgoing().get())
			out << "\n" << out_it->str ;
		//incoming
		out << "\n[" << (it->second.isIncomingDirty()? "IDT" : "IND") << "] size[" << it->second.isIncomingDirty() << "]";
		DATA_MSG_PTR in_it;
		BOOST_FOREACH(in_it, it->second.getIncoming().get())
			out << "\n" << in_it->str ;
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
	{
		std::cout << "currWorker dead" << std::endl;
		return true;
	}
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
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it, it_end(subscriptionList.end());
	for(it = subscriptionList.begin(); it != it_end; it++)
	{
		const sim_mob::Entity * target = (*it).first;
		//you or your worker are probably dead already. you just don't know it
		if (!target->currWorker)
			{
				unSubscribeEntity(target);
				continue;
			}
		const std::vector<sim_mob::Entity*> & managedEntities_ = (target->currWorker)->getEntities();
		std::vector<sim_mob::Entity*>::const_iterator  it_entity = std::find(managedEntities_.begin(), managedEntities_.end(), target);
		if(it_entity == managedEntities_.end())
		{
			unSubscribeEntity(target);
			continue;
		}
		else
		{
//			std::cout << std::dec << "_Agent [" << target << ":" << *it_entity << "] is still among " << (int)((target->currWorker)->getEntities().size()) << " entities of worker[" << target->currWorker << "]" << std::endl;
		}
	}
}
//send the buffer, when done, release the elements of the buffer.
//this is safe coz the datacontainer in the argument list is 'copied', no pointer, no reference
void NS3_Communicator::bufferSend(DataContainer &tempSendBuffer)
{
	std::cout << "inside NS3_Communicator::bufferSend(" << tempSendBuffer.get().size() << ")" << std::endl;
	commImpl.send(tempSendBuffer);
	tempSendBuffer.reset();
}
bool NS3_Communicator::allAgentUpdatesDone()
{
//	WriteLock(*NS3_Communicator_Mutex);
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::const_iterator it, it_end(subscriptionList.end());

	for(it = subscriptionList.begin(); it != it_end; it++)
	{

			sim_mob::CommunicationSupport & info = subscriptionList.at(it->first);//easy read
//			if(deadEntityCheck(info))
//			{
//				throw std::runtime_error("You are dealing with a probably dead entity");
//			}
			try
			{
				if (info.isAgentUpdateDone())
				{
					WriteLock(*NS3_Communicator_Mutex);
					duplicateEntityDoneChecker.insert(it->first);
				}
			}
			catch(char * str)
			{
				if(deadEntityCheck(info))
				{
					unSubscribeEntity(info);
				}
				else
					throw std::runtime_error("Unknown Error checking entity");
			}
	}
	return(duplicateEntityDoneChecker.size() >= subscriptionList.size());
}
void NS3_Communicator::trySend(timeslice now)
{

	if(!sendBuffer.empty())
	{
		DataContainer tempSendBuffer;
		std::cout << now.frame() << ": sendBuffer NOT empty[" << sendBuffer.get().size() << "] =>firing bufferSend " << std::endl;
		{
			WriteLock(*NS3_Communicator_Mutex);
			//don't panic. this is not a heavy copy
			//coz the datacontainer's buffer just keeps the pointers
			tempSendBuffer = sendBuffer;
			sendBuffer.clear();
//				std::cout << "tempSendBuffer.size(" << tempSendBuffer.get().size() << ")" << std::endl;
		}
		bufferSend(tempSendBuffer);
//			todo may be nobody needs a lock in this function but how about other functions
		tempSendBuffer.reset();
	}
	else
		std::cout << now.frame() << "sendBuffer IS empty()" << std::endl;
}
//todo: think of a better return value than just void
bool NS3_Communicator::processOutgoingData(timeslice now)
{
	int i = 0;
	duplicateEntityDoneChecker.clear();
	do
	{
		std::cout /*<< std::dec*/ << now.frame() << "  NS3_Communicator::processOutgoingData iteration[" << i++ << "]" << std::endl;
		trySend(now);
		refineSubscriptionList();
	}while(!allAgentUpdatesDone());
	trySend(now);//the last chance for the leftovers
	std::cout<< "  NS3_Communicator::processOutgoingData [" << now.frame() << "]  allAgentUpdatesDone"  << std::endl;
}

//todo: think of a better return value than just void
void NS3_Communicator::processIncomingData(timeslice now)
{
  DataContainer &receiveBatch = receiveBuffer; //will contain data belonging to many agents
//  const commResult &res = commImpl.receive(receiveBatch);//not working now
//  if(res.getResult() != commResult::success) return;
  if(!receiveBatch.empty())
  {
	  std::cout << "NS3_Communicator::processIncomingData=>RECEIVED DATA FROM THE PEER" << std::endl;
  }
  else
  {
	  std::cout << "Incoming is empty" << std::endl;
  }
  DATA_MSG_PTR it;
  //distribute among agents
  BOOST_FOREACH(it,receiveBatch.get())
  {
	  //get the reference to the receiving agent's subscription record
	  sim_mob::Entity * receiver = (sim_mob::Entity *) (it->receiver);
	  try
	  {
		  sim_mob::CommunicationSupport & info = subscriptionList.at(receiver);
		  std::cout << (*it).str.size() << std::endl;
		  std::cout << (*it).str << std::endl;
		  //the receiving agent's subscription record has a reference to its incoming buffer
		  info.addIncoming(it);
	  }
	  catch(std::out_of_range e)
	  {
		  unSubscribeEntity(receiver);
	  }
  }
  receiveBatch.clear();

}

void NS3_Communicator::reset()
{
//	WriteLock(*NS3_Communicator_Mutex);
	std::map<const sim_mob::Entity*,sim_mob::CommunicationSupport&>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		sim_mob::CommunicationSupport &info = it->second;
		info.reset();

	}
	sendBuffer.reset();//data must have already been sent by now. no need to keep them.
	receiveBuffer.clear();//reset will delete data wich now belongs to agents. agent will delete the data as and when they want

}
void NS3_Communicator::reset(sim_mob::CommunicationSupport &info)
{
	info.reset();
}

boost::shared_ptr<Lock>  NS3_Communicator::subscribeEntity(sim_mob::CommunicationSupport & value)
{
	{
//		WriteLock(*NS3_Communicator_Mutex);
		subscriptionList.insert(std::pair<const sim_mob::Entity*, sim_mob::CommunicationSupport &>(&(value.getEntity()),value));
		std::cout << "Subscribed agent[" << &value.getEntity() << "] with outgoing[" << &(subscriptionList.at(&value.getEntity()).getOutgoing()) << ":" << &sendBuffer << "]" << std::endl;
	}
	return NS3_Communicator_Mutex;
}
bool  NS3_Communicator::unSubscribeEntity(sim_mob::CommunicationSupport &value)
{
//	WriteLock(*NS3_Communicator_Mutex);
	return subscriptionList.erase(&value.getEntity());
}

bool  NS3_Communicator::unSubscribeEntity(const sim_mob::Entity * agent)
{

	WriteLock(*NS3_Communicator_Mutex);
	subscriptionList.erase(agent);
	//also refine duplicateEntityDoneChecker
	std::set<const sim_mob::Entity*>::iterator it = duplicateEntityDoneChecker.find(agent);
	if(it != duplicateEntityDoneChecker.end() ) {
		duplicateEntityDoneChecker.erase(it);
	}
	return false;//for now. I dont know if ne1 needs its return value
}

sim_mob::DataContainer &NS3_Communicator::getSendBuffer()
{
	//		WriteLock(*NS3_Communicator_Mutex);
	return sendBuffer;
}

void NS3_Communicator::popReceiveBuffer(DATA_MSG_PTR & value)
{
	//	WriteLock(*NS3_Communicator_Mutex);
	value = receiveBuffer.get().front();
	receiveBuffer.get().erase(receiveBuffer.get().begin());
}

sim_mob::DataContainer &NS3_Communicator::getReceiveBuffer()
{
	//	WriteLock(*NS3_Communicator_Mutex);
	return receiveBuffer;
}
//void NS3_Communicator::addSendBuffer(sim_mob::DATA_MSG_PTR &value){
//	sim_mob::WriteLock Lock(*myLock);
//	sendBuffer.add(value);
//}
void NS3_Communicator::addSendBuffer(sim_mob::DataContainer &value){
	//	WriteLock(*NS3_Communicator_Mutex);
	sendBuffer.add(value);
}
void NS3_Communicator::addSendBuffer(std::vector<DATA_MSG_PTR> &values){
	//		WriteLock(*NS3_Communicator_Mutex);
	sendBuffer.add(values);
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
