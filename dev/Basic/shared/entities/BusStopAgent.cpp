//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusStopAgent.hpp"

#include "entities/Person.hpp"
#include "entities/AuraManager.hpp"
#include "geospatial/BusStop.hpp"
#include "workers/WorkGroup.hpp"
#include "entities/roles/activityRole/WaitBusActivityRole.hpp"
#include "entities/roles/activityRole/WaitBusActivityRoleFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

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
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	uint32_t currMS = (config.simStartTime() + DailyTime(now.ms())).offsetMS_From(DailyTime("00:00:00"));// transfered to ms based on midnight
	if(now.frame() == 108000) {
		buslineId_HeadwayGapMSs.clear();

		std::stringstream currReachedMSOut;
		std::map<std::string, std::vector<uint32_t> >::const_iterator it;
		for (it = buslineId_CurrReachedMSs.begin(); it != buslineId_CurrReachedMSs.end(); ++it) {
			// #print 857_1 information
			if((it->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "currReachedMsInformation for buslineId " << (it->first) << std::endl;
				for(int i = 0; i < it->second.size(); i++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "at stop " << this->busstop_.busstopno_ << " "
									 << (it->first) << " "
									 << (it->second)[i] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
//				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

		for (it = buslineId_AlightingNum.begin(); it != buslineId_AlightingNum.end(); ++it) {
			// #print 857_1 information
			if((it->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "AlightingInformation for buslineId " << (it->first) << std::endl;
				for(int j = 0; j < it->second.size(); j++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "alightingNum " << this->busstop_.busstopno_ << " "
									 << (it->first) << " "
									 << (it->second)[j] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
//				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

		for (it = buslineId_BoardingNum.begin(); it != buslineId_BoardingNum.end(); ++it) {
			// #print 857_1 information
			if((it->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "BoardingInformation for buslineId " << (it->first) << std::endl;
				for(int k = 0; k < it->second.size(); k++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "boardingNum " << this->busstop_.busstopno_ << " "
									 << (it->first) << " "
									 << (it->second)[k] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
//				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

		std::map<std::string, std::vector<double> >::const_iterator it1;
		for (it1 = buslineId_BoardingAlightingSecs.begin(); it1 != buslineId_BoardingAlightingSecs.end(); ++it1) {
			// #print 857_1 information
			if((it1->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "BoardingAlightingSecs_Information for buslineId " << (it1->first) << std::endl;
				for(int m = 0; m < it1->second.size(); m++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "boardingAlightingSecs " << this->busstop_.busstopno_ << " "
									 << (it1->first) << " "
									 << (it1->second)[m] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
//				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

		std::map<std::string, std::vector<int> >::const_iterator it2;
		for (it2 = buslineId_bustripRunSequenceNums.begin(); it2 != buslineId_bustripRunSequenceNums.end(); ++it2) {
			// #print 857_1 information
			if((it2->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "BusTripRunSequenceNumInformation for buslineId " << (it2->first) << std::endl;
				for(int n = 0; n < it2->second.size(); n++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "bustripRunSequenceNum " << this->busstop_.busstopno_ << " "
									 << (it2->first) << " "
									 << (it2->second)[n] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
//				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

		for (it2 = buslineId_passengerCounts.begin(); it2 != buslineId_passengerCounts.end(); ++it2) {
			// #print 857_1 information
			if((it2->first) == "857_1") {
//				currReachedMSOut << now.frame() << ","
//								 << this->busstop_.busstopno_ << ","
//								 << (it->first) << ","
//								 << (it->second).size() << std::endl;
				currReachedMSOut << "PassengerCountsInformation for buslineId " << (it2->first) << std::endl;
				for(int s = 0; s < it2->second.size(); s++) {
	//				currReachedMSOut << " (\"currReachedMS\""
	//								<< ": " << (it->second)[i];
	//				currReachedMSOut << "\"})" << std::endl;
					currReachedMSOut << now.frame() << " "
									 << "passengerCounts " << this->busstop_.busstopno_ << " "
									 << (it2->first) << " "
									 << (it2->second)[s] << std::endl;
//					currReachedMSOut << std::endl;
				}
				currReachedMSOut << std::endl;
				HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();
			}
		}

// #print all busline information
//		for (it = buslineId_CurrReachedMSs.begin(); it != buslineId_CurrReachedMSs.end(); ++it) {
//			currReachedMSOut << "(\"BusStopAgent\""
//							<<" frame no:" << now.frame()
//							<< " bus stop no:" << this->busstop_.busstopno_ << std::endl;
//			currReachedMSOut << " (\"buslineID\""
//							<< ": " << (it->first)
//							<< " currReachedMS size: " << (it->second).size()
//							<< std::endl;
//			for(int i = 0; i < it->second.size(); i++) {
////				currReachedMSOut << " (\"currReachedMS\""
////								<< ": " << (it->second)[i];
////				currReachedMSOut << "\"})" << std::endl;
//				currReachedMSOut << (it->second)[i];
//				currReachedMSOut << std::endl;
//			}
//			currReachedMSOut << std::endl;
//		}
//		HeadwayAtBusStopInfoPrint() << currReachedMSOut.str();




// #print headway gaps
//		std::map<std::string, std::vector<uint32_t> >::const_iterator iter;
//		std::stringstream headwayGapMSOut;
//		for (iter = buslineId_CurrReachedMSs.begin(); iter != buslineId_CurrReachedMSs.end(); ++iter) {
//			if(iter->second.size() >= 2) {
////				headwayGapMSOut << "(\"BusStopAgent\""
////								<<" frame no:" << now.frame()
////								<< " bus stop no:" << this->busstop_.busstopno_ << std::endl;
//				for(int i = 0; i < iter->second.size() - 1; i++) {
////					headwayGapMSOut << "iter->second[i+1]: " << iter->second[i+1]
////					                << "iter->second[i]: " << iter->second[i]
////					                << std::endl;
//					buslineId_HeadwayGapMSs[iter->first].push_back(iter->second[i+1] - iter->second[i]);
//				}
//			}
//		}
//
////		std::stringstream headwayGapMSOut;
//		for(iter = buslineId_HeadwayGapMSs.begin(); iter != buslineId_HeadwayGapMSs.end(); ++iter) {
//			headwayGapMSOut << "(\"BusStopAgent\""
//							<<" frame no:" << now.frame()
//							<< " bus stop no:" << this->busstop_.busstopno_ << std::endl;
//			headwayGapMSOut << " (\"buslineID\""
//							<< ": " << (iter->first)
//							<< " headway size: " << (iter->second).size()
//							<< std::endl;
//			for(int i = 0; i < iter->second.size(); i++) {
//				headwayGapMSOut << " (\"headwayGap\""
//								<< ": " << (iter->second)[i];
//				headwayGapMSOut << "\"})" << std::endl;
//			}
//			headwayGapMSOut << std::endl;
//		}
//		HeadwayAtBusStopInfoPrint() << headwayGapMSOut.str();
//		HeadwayAtBusStopInfoPrint() << std::endl;
	}

}

Entity::UpdateStatus sim_mob::BusStopAgent::frame_tick(timeslice now)
{
	if(now.ms() % 5000 == 0) {// every 5000ms check AuraManager
		vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(Point2D((busstop_.xPos - 3500),(busstop_.yPos - 3500)),Point2D((busstop_.xPos + 3500),(busstop_.yPos + 3500)), this);
		for (vector<const Agent*>::iterator it = nearby_agents.begin();it != nearby_agents.end(); it++)
		{
			//Retrieve only agents with WaitBusActivityRoles.
		 	const Person* person = dynamic_cast<const Person *>(*it);
		 	Person* p = const_cast<Person *>(person);
		 	WaitBusActivityRole* waitbusactivityRole = p ? dynamic_cast<WaitBusActivityRole*>(p->getRole()) : nullptr;
		 	if(waitbusactivityRole) {
		 		WaitBusActivityRoleMovement* waitbusactivityRoleMovement = dynamic_cast<WaitBusActivityRoleMovement*> (waitbusactivityRole->Movement());
		 		if((!waitbusactivityRoleMovement->getRegisteredFlag()) && (waitbusactivityRoleMovement->getBusStopAgent() == this)) {// not registered and waiting in this BusStopAgent
		 			boarding_WaitBusActivities.push_back(waitbusactivityRole);
		 			waitbusactivityRoleMovement->setRegisteredFlag(true);// set this person's role to be registered
		 		}
		 	}
		}
		if(!boarding_WaitBusActivities.empty())
			sort(boarding_WaitBusActivities.begin(),boarding_WaitBusActivities.end(),less_than_TimeOfReachingBusStop());
//		for(int i = 0; i < boarding_WaitBusActivities.size(); i++) {
//			WaitBusActivityRoleMovement* waitbusactivityRoleMovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_WaitBusActivities[i]->Movement());
////			if(waitbusactivityRoleMovement->boarding_MS == -1) {
////				boarding_WaitBusActivities.erase(boarding_WaitBusActivities.begin() + i);
////			}
//			if(waitbusactivityRoleMovement->isBoarded) {
//				boarding_WaitBusActivities.erase(boarding_WaitBusActivities.begin() + i);
//			}
//		}
	}
	for(int i = 0; i < boarding_WaitBusActivities.size(); i++) {
		WaitBusActivityRoleMovement* waitbusactivityRoleMovement = dynamic_cast<WaitBusActivityRoleMovement*> (boarding_WaitBusActivities[i]->Movement());
//			if(waitbusactivityRoleMovement->boarding_MS == -1) {
//				boarding_WaitBusActivities.erase(boarding_WaitBusActivities.begin() + i);
//			}
		if(waitbusactivityRoleMovement->isBoarded) {
			boarding_WaitBusActivities.erase(boarding_WaitBusActivities.begin() + i);
		}
	}
	//unregisterAlightedPerons();// check the Alighted Queue and unregister Alighted Persons

	return Entity::UpdateStatus::Continue;
}

void sim_mob::BusStopAgent::unregisterAlightedPerons()
{
	for(int i = 0; i < alighted_Persons.size(); i++) {
		if(!(alighted_Persons[i]->currWorkerProvider)) {
			std::cout << "alighted_Persons[i]->getRole(): " << alighted_Persons[i]->getRole() << std::endl;
			alighted_Persons.erase(alighted_Persons.begin() + i);// ghost, erase this person from the BusStopAgent
		} else {
			if(!alighted_Persons[i]->findPersonNextRole())// find and assign the nextRole to this Person, when this nextRole is set to be nullptr?
			{
				std::cout << "End of trip chain...." << std::endl;
			}
			if(alighted_Persons[i]->getNextRole()->roleType == Role::RL_PEDESTRIAN) {// if roleType belong to Pedestrian type
				alighted_Persons.erase(alighted_Persons.begin() + i);// erase this person from the BusStopAgent
			}
		}
	}
}
