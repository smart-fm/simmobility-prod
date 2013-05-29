/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/AuraManager.hpp"
#include "workers/WorkGroup.hpp"

using std::vector;
using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;

vector<BusStopAgent*> BusStopAgent::all_BusstopAgents_;

size_t sim_mob::BusStopAgent::AllBusStopAgentsCount()
{
	return all_BusstopAgents_.size();
}

void sim_mob::BusStopAgent::AssignAllBusStopAgents(sim_mob::WorkGroup& wg)
{
	for (vector<BusStopAgent*>::iterator it=all_BusstopAgents_.begin(); it!=all_BusstopAgents_.end(); it++) {
		wg.assignAWorker(*it);
	}
}

void sim_mob::BusStopAgent::RegisterNewBusStopAgent(BusStop& busstop, const MutexStrategy& mtxStrat)
{
	BusStopAgent* sig_ag = new BusStopAgent(busstop, mtxStrat);
	sig_ag->setBusStopAgentNo(busstop.getBusstopno_());
	busstop.generatedBusStopAgent = sig_ag;
	all_BusstopAgents_.push_back(sig_ag);
}

bool sim_mob::BusStopAgent::HasBusStopAgents()
{
	return !all_BusstopAgents_.empty();
}

bool sim_mob::BusStopAgent::isNonspatial()
{
	return true;
}

void sim_mob::BusStopAgent::PlaceAllBusStopAgents(std::vector<sim_mob::Entity*>& agents_list)
{
	std::cout << "all_BusStopAgents size: " << all_BusstopAgents_.size() << std::endl;
	//Push every BusStopAgent on the list into the agents array as an active agent
	for (vector<BusStopAgent*>::iterator it=all_BusstopAgents_.begin(); it!=all_BusstopAgents_.end(); it++) {
		agents_list.push_back(*it);
	}
}

BusStopAgent* sim_mob::BusStopAgent::findBusStopAgentByBusStop(const BusStop* busstop)
{
	for (int i=0; i<all_BusstopAgents_.size(); i++) {
		if(all_BusstopAgents_[i]->getBusStop().getBusstopno_() == busstop->getBusstopno_()) {
			return all_BusstopAgents_[i];
		}
	}

	return nullptr;
}

BusStopAgent* sim_mob::BusStopAgent::findBusStopAgentByBusStopNo(const std::string& busstopno)
{
	for (int i=0; i<all_BusstopAgents_.size(); i++) {
		if(all_BusstopAgents_[i]->getBusStop().getBusstopno_() == busstopno) {
			return all_BusstopAgents_[i];
		}
	}

	return nullptr;
}

void sim_mob::BusStopAgent::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
	Agent::buildSubscriptionList(subsList);
}

bool sim_mob::BusStopAgent::frame_init(timeslice now)
{
	return true;
}

void sim_mob::BusStopAgent::frame_output(timeslice now)
{
	if(now.ms() % 5000 == 0) {// every 5000ms output
		LogOut("(\"BusStopAgent\""
			<<","<<now.frame()
			<<","<<getId()
			<<",{" <<"\"BusStopAgent no\":\""<<busstopAgentno_
			<<"\"})"<<std::endl);
	}
}

Entity::UpdateStatus sim_mob::BusStopAgent::frame_tick(timeslice now)
{
	if(now.ms() % 5000 == 0) {// every 5000ms check AuraManager
		vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((busstop_.xPos - 3500),(busstop_.yPos - 3500)),Point2D((busstop_.xPos + 3500),(busstop_.yPos + 3500)));
		std::cout << "nearby_agents size: " << nearby_agents.size() << std::endl;
		for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
		{
			//Retrieve only agents with WaitBusActivityRoles.
		 	const Person* person = dynamic_cast<const Person *>(*it);
		 	Person* p = const_cast<Person *>(person);
		 	WaitBusActivityRole* waitbusactivityRole = p ? dynamic_cast<WaitBusActivityRole*>(p->getRole()) : nullptr;
		 	if(waitbusactivityRole) {
		 		if((!waitbusactivityRole->getRegisteredFlag()) && (waitbusactivityRole->getBusStopAgent() == this)) {// not registered and waiting in this BusStopAgent
		 			boarding_WaitBusActivities.push_back(waitbusactivityRole);
		 			waitbusactivityRole->setRegisteredFlag(true);// set this person's role to be registered
		 		}
		 	}
		}
		if(!boarding_WaitBusActivities.empty())
			sort(boarding_WaitBusActivities.begin(),boarding_WaitBusActivities.end(),less_than_TimeOfReachingBusStop());
	}
	//unregisterAlightedPerons();// check the Alighted Queue and unregister Alighted Persons

	return Entity::UpdateStatus::Continue;
}

void sim_mob::BusStopAgent::unregisterAlightedPerons()
{
	for(int i = 0; i < alighted_Persons.size(); i++) {
		if(!(alighted_Persons[i]->currWorker)) {// currWorker is nullptr
			std::cout << "alighted_Persons[i]->getRole(): " << alighted_Persons[i]->getRole() << std::endl;
			alighted_Persons.erase(alighted_Persons.begin() + i);// ghost, erase this person from the BusStopAgent
		} else {
			if(!alighted_Persons[i]->findPersonNextRole())// find and assign the nextRole to this Person, when this nextRole is set to be nullptr?
			{
				std::cout << "End of trip chain...." << std::endl;
			}
			Pedestrian2* pedestrian2 = dynamic_cast<Pedestrian2*> (alighted_Persons[i]->getNextRole());// ? nextRole or currRole
			if(pedestrian2) {// nextRole is pedestrian
				//pedestrian2->setAtBusStop(false);
				alighted_Persons.erase(alighted_Persons.begin() + i);// erase this person from the BusStopAgent
			}
		}
	}
}
