/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <algorithm>
#include "SegmentVehicles.hpp"

namespace sim_mob {
	sim_mob::SegmentVehicles::SegmentVehicles(const sim_mob::RoadSegment* rdSeg) : roadSegment(rdSeg) {
	}

	void SegmentVehicles::addQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		queuingAgents[lane].push_back(ag);
	}

	void SegmentVehicles::removeQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		std::vector<sim_mob::Agent*>::iterator queueIt =  std::find(queuingAgents[lane].begin(), queuingAgents[lane].end(), ag);
		if(queueIt != queuingAgents[lane].end()){
			queuingAgents[lane].erase(queueIt);
		}
	}

	void SegmentVehicles::addMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		movingAgents[lane].insert(ag);
	}

	void SegmentVehicles::removeMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		movingAgents[lane].erase(ag);
	}

	std::set<Agent*> SegmentVehicles::getAgentsOnMovingVehicles(const sim_mob::Lane* lane) {
		return movingAgents[lane];
	}

	std::vector<Agent*> SegmentVehicles::getAgentsOnQueuingVehicles(const sim_mob::Lane* lane) {
		return queuingAgents[lane];
	}

	const sim_mob::RoadSegment* sim_mob::SegmentVehicles::getRoadSegment() const {
		return roadSegment;
	}

	bool SegmentVehicles::isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent) {
		return agent == queuingAgents[lane].front();
	}

	sim_mob::Agent* SegmentVehicles::dequeue(const sim_mob::Lane* lane){
		sim_mob::Agent* ag = queuingAgents[lane].front();
		queuingAgents[lane].erase(queuingAgents[lane].begin());
		return ag;
	}

	void sim_mob::SegmentVehicles::merge(sim_mob::SegmentVehicles* segVehicles) {

		const sim_mob::RoadSegment* rdSeg = nullptr;
		if (this->getRoadSegment() == segVehicles->getRoadSegment())
		{

			rdSeg = this->getRoadSegment();

			//for each lane in the rdSeg
			for (std::vector<sim_mob::Lane*>::const_iterator laneIt =
					rdSeg->getLanes().begin(); laneIt != rdSeg->getLanes().end(); laneIt++)
			{
				//union movingAgents set
				std::set<sim_mob::Agent*> segVehMovingAgentsOnLane = segVehicles->getAgentsOnMovingVehicles(*laneIt);
				this->getAgentsOnMovingVehicles(*laneIt).insert(segVehMovingAgentsOnLane.begin(), segVehMovingAgentsOnLane.end());

				//merge queuing vehicles
				std::vector<sim_mob::Agent*> segVehQueuingAgentsOnLane = segVehicles->getAgentsOnQueuingVehicles(*laneIt);
				std::vector<sim_mob::Agent*> thisQueuingAgentsOnLane = this->getAgentsOnQueuingVehicles(*laneIt);
				std::vector<sim_mob::Agent*> mergedQueuingAgentsOnLane;

				sim_mob::Agent* a = nullptr;
				sim_mob::Agent* b = nullptr;
				while (thisQueuingAgentsOnLane.size() + segVehQueuingAgentsOnLane.size() > 0) {
					if(!a && !thisQueuingAgentsOnLane.empty()) {
						a = thisQueuingAgentsOnLane.front();
						thisQueuingAgentsOnLane.erase(thisQueuingAgentsOnLane.begin());
					}
					if(!b && !segVehQueuingAgentsOnLane.empty()) {
						b = segVehQueuingAgentsOnLane.front();
						segVehQueuingAgentsOnLane.erase(segVehQueuingAgentsOnLane.begin());
					}

					if(a && b){
						if(a->enqueueTick <= b->enqueueTick){
							mergedQueuingAgentsOnLane.push_back(a);
							a = nullptr;
						}
						else {
							mergedQueuingAgentsOnLane.push_back(b);
							b = nullptr;
						}
					}
					else if(a){
						mergedQueuingAgentsOnLane.push_back(a);
						a = nullptr;
					}
					else if(b){
						mergedQueuingAgentsOnLane.push_back(b);
						b = nullptr;
					}
				} // end of while loop

				//Set the merged queue back to this SegmentVehicles object
				queuingAgents.erase(*laneIt);
				queuingAgents.insert(std::make_pair(*laneIt, mergedQueuingAgentsOnLane));
			} // end of for loop
		} // end of if
	}

} // end of namespace sim_mob
