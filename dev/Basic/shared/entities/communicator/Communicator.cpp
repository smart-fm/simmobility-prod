#include "Communicator.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include <boost/thread/mutex.hpp>


///
namespace sim_mob {

//sim_mob::All_NS3_Communicators sim_mob::NS3_Communicator::all_NS3_cummunication_agents;
NS3_Communicator::NS3_Communicator(const MutexStrategy& mtxStrat, int id )
: Agent(mtxStrat, id)
{

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
	std::cout << "communicator tick:"<< now.frame() << " ================================================\n";
	std::map<const sim_mob::Entity*,sim_mob::subscriptionInfo>::iterator it_1;
	//thread-1 (send):
	processOutgoingData(now);
//	commImpl.shortCircuit();
//	thread-2 (receive):
	processIncomingData(now);
//	thread.join()
	reset();
	std::cout <<"------------------------------------------------------\n";
	return UpdateStatus(UpdateStatus::RS_CONTINUE);

}
void NS3_Communicator::printSubscriptionList(timeslice now)
{
	std::ostringstream out("");
	out << "printSubscriptionList\n" ;
	std::map<const sim_mob::Entity*,sim_mob::subscriptionInfo>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		out.str("");
		out << "communicator tick:" << now.frame() << ": [" << it->first <<"]";
		out << "[" << it->second.isAgentUpdateDone() <<  ":" << it->second.isOutgoingDirty() << "] out[" << &(it->second.getOutgoing()) << " : size " << it->second.getOutgoing().get().size() << "]";
		//outgoing
		DATA_MSG_PTR out_it;
		BOOST_FOREACH(out_it, it->second.getOutgoing().get())
			out << " " << out_it->str ;
		out << std::endl;
		std::cout << out.str();
	}
}
bool NS3_Communicator::deadEntityCheck(sim_mob::subscriptionInfo & info) {
	//some top notch optimizasion! to check if the agent is alive at all?
	info.cnt_1++;
	if (info.cnt_1 < 1000)
		return false;

	//you or your worker are probably dead already. you just don't know it
	if (!(info.getEntity()->currWorker))
		return true;
	//one more check to see if the entity is deleted
	const std::vector<sim_mob::Entity*> & managedEntities_ = info.getEntity()->currWorker->getEntities();
	std::vector<sim_mob::Entity*>::const_iterator it = managedEntities_.begin();
	for (std::vector<sim_mob::Entity*>::const_iterator it =	managedEntities_.begin(); it != managedEntities_.end(); it++)
		if (*it == info.getEntity()) 	return true;

	return false;
}
//todo: think of a better return value than just void
bool NS3_Communicator::processOutgoingData(timeslice now)
{
	int totalPackets = 0;
	printSubscriptionList(now);
	std::map<const sim_mob::Entity*,sim_mob::subscriptionInfo>::iterator it;
	bool allUpdatesAreDone = true; //used to get out this loop
	bool nothingToSend = true;
	DataContainer sendBatch;
	//following container are just to avoid unneseccary searches
	std::set<const sim_mob::Entity*> duplicateEntityDoneChecker ;
	nothingToSend = true;
	sendBatch.reset();
	while(1)
	{
		std::cout << "\nWhile\n";
		if(subscriptionList.size() < 1) {
			std::cout << "\nWhile-break-1\n";
			break;
		}


		//inner iteration find those who have something to send, batch them up and send them
		for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
		{
			//if you see the agent is removed from the simulation, it is best to remove it from the list and break the inner loop
			sim_mob::subscriptionInfo & info = subscriptionList.at(it->first);//easy read

			//filter the iteration
			if (duplicateEntityDoneChecker.count(it->first) > 0)//already done
			{
				std::cout << "[" << info.getEntity() << "] 1:duplicate entry\n";
				continue;
			}

			//you don't do anything untill agent's update is done
			if (info.isAgentUpdateDone() == false)
				{
					if(deadEntityCheck(info))
					{
						subscriptionList.erase(info.getEntity());
						break; //u just messed up the subscriptionList's irerator so get out of this loop and come back later
					}
					std::cout << "[" << info.getEntity() << "] 2: agentUpdate not Done \n";
					continue;
				} //update not completed
			else
				duplicateEntityDoneChecker.insert(it->first); //already done

			//now that agent update is done, see if it has anything to send
			if (info.isOutgoingDirty() == false)
			{
				std::cout << "[" << info.getEntity() << "] 3:outgoing Is Not Dirty\n";
				continue;
			} //no data

			//if your program raches this point in this inner loop,, it means some data is available for sending
			{
				std::cout <<"\n outgoing=>Tick:" << now.frame() << "agent[" << info.getEntity() << "] update[" << info.isAgentUpdateDone() << "] dirty[" << info.isOutgoingDirty() <<"]\n";
				nothingToSend = false;
				sendBatch.add(info.getOutgoing());
			}

		}//for

		if(duplicateEntityDoneChecker.size() >= subscriptionList.size())  //used >= to be safe. coz you might have inserted the agent into duplicatechecker and agent is killed immediately....
		{
			std::cout << "\nWhile-break-(All updates are done)\n";
			break;
		};//todo what if an entity is set for removal? then no updatDone->infinite loop

	}//while: when you get out of this loop that this tick is over wrt outgoing
	std::cout << "\nWhile--done\n";

	if(!nothingToSend)
		{
			totalPackets += sendBatch.get().size();
			std::cout << "Sending batch of size " << sendBatch.get().size() << " to commImpl.send(sendBatch)" << std::endl;
			commImpl.send(sendBatch);
		}
		else
			std::cout << "Nothing to send\n";
	//todo remove or modify this later
	reset();
}
//todo: think of a better return value than just void
void NS3_Communicator::processIncomingData(timeslice now)
{
	if(now.frame() != 4)
	{
		return;
		std::cout << std::endl;
	}
  DataContainer receiveBatch; //will contain data belonging to many agents
  const commResult &res = commImpl.receive(receiveBatch);
  if(res.getResult() != commResult::success) return;
  DATA_MSG_PTR it;
  //distribute among agents
  BOOST_FOREACH(it,receiveBatch.get())
  {
	  //get the reference to the receiving agent's subscription record
	  sim_mob::Entity * receiver = (sim_mob::Entity *) (it->receiver);
	  sim_mob::subscriptionInfo & info = subscriptionList.at(receiver);
	  //the receiving agent's subscription record has a reference to its incoming buffer
	  info.addIncoming(it);
  }

}

void NS3_Communicator::reset()
{
	std::map<const sim_mob::Entity*,sim_mob::subscriptionInfo>::iterator it;
	for(it = subscriptionList.begin(); it != subscriptionList.end(); it++)
	{
		subscriptionInfo &info = it->second;
		info.reset();

	}

}
void NS3_Communicator::reset(subscriptionInfo &info)
{
	info.reset();
}

void  NS3_Communicator::subscribeEntity(subscriptionInfo value)
{
	//a simple check before adding
	if(!value.getEntity())
		{
			throw std::runtime_error("Agent data is essential\n");
		}

	subscriptionList.insert(std::make_pair(value.getEntity(), value));
	std::cout << "Subscribed agent[" << value.getEntity() << "] with outgoing[" << &(subscriptionList.at(value.getEntity()).getOutgoing()) << "]" << std::endl;
}

bool  NS3_Communicator::unSubscribeEntity(subscriptionInfo value)
{
	return subscriptionList.erase(value.getEntity());
}

bool  NS3_Communicator::unSubscribeEntity(const sim_mob::Entity * agent)
{
	return subscriptionList.erase(agent);
}

sim_mob::DataContainer &NS3_Communicator::getSendBuffer()
{
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
