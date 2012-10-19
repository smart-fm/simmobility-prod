/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <algorithm>
#include "AgentKeeper.hpp"

namespace sim_mob {

	AgentKeeper::AgentKeeper(const sim_mob::RoadSegment* rdSeg)
		: roadSegment(rdSeg)
	{
		// initialize LaneAgents in the map
		std::vector<sim_mob::Lane*>::const_iterator lane = rdSeg->getLanes().begin();
		while(lane != rdSeg->getLanes().end())
		{
			laneAgentsMap.insert(std::make_pair(*lane, new sim_mob::LaneAgents()));
			lane++;
		}
	}


	void AgentKeeper::addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneAgentsMap[lane]->addAgent(ag);
	}

	void AgentKeeper::removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneAgentsMap[lane]->removeAgent(ag);
	}

	sim_mob::Agent* AgentKeeper::dequeue(const sim_mob::Lane* lane) {
		return laneAgentsMap[lane]->dequeue();
	}

	std::vector<sim_mob::Agent*> AgentKeeper::getAgents(const sim_mob::Lane* lane) {
		return laneAgentsMap[lane]->laneAgents;
	}

	std::pair<int, int> AgentKeeper::getLaneAgentCounts(const sim_mob::Lane* lane) {
		return std::make_pair(
				laneAgentsMap[lane]->getQueuingAgentsCount(),
				laneAgentsMap[lane]->getMovingAgentsCount()
				);
	}

	std::map<sim_mob::Lane*, std::pair<int, int> > AgentKeeper::getAgentCountsOnLanes() {
		std::map<sim_mob::Lane*, std::pair<int, int> > agentCountsOnLanes;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			agentCountsOnLanes.insert(std::make_pair(*lane, getLaneAgentCounts(*lane)));
			lane++;
		}
		return agentCountsOnLanes;
	}

	const sim_mob::RoadSegment* sim_mob::AgentKeeper::getRoadSegment() const {
		return roadSegment;
	}

	bool AgentKeeper::isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent) {
		return (agent == laneAgentsMap[lane]->laneAgents.front());
	}

	unsigned int AgentKeeper::numMovingInSegment() {
		int movingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			movingCounts = movingCounts + laneAgentsMap[*lane]->getMovingAgentsCount();
			lane++;
		}
		return movingCounts;
	}

	unsigned int AgentKeeper::numQueueingInSegment() {
		int queuingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			queuingCounts = queuingCounts + laneAgentsMap[*lane]->getQueuingAgentsCount();
			lane++;
		}
		return queuingCounts;
	}

	bool AgentKeeper::allAgentsProcessed() {
		bool allAgentsProcessed = true;
		std::map<const sim_mob::Lane*, sim_mob::Agent* >::iterator i = frontalAgents.begin();
		while(i!=frontalAgents.end()) {
			if((*i).second != nullptr) {
				allAgentsProcessed = false;
				break;
			}
		}
		return allAgentsProcessed;
	}

	sim_mob::Agent* AgentKeeper::agentClosestToStopLine() {
		std::map<const sim_mob::Lane*, sim_mob::Agent* >::iterator i = frontalAgents.begin();
		sim_mob::Agent* ag = nullptr;
		const sim_mob::Lane* agLane = nullptr;
		double minDistance = std::numeric_limits<double>::infinity();
		while(i!=frontalAgents.end()) {
			if((*i).second != nullptr) {
				if(minDistance == (*i).second->distanceToEndOfSegment) {
					// If current ag and (*i) are at equal distance to the stop line, we toss a coin and choose one of them
					bool coinTossResult = ((rand() / (double)RAND_MAX) < 0.5);
					if(coinTossResult) {
						agLane = (*i).first;
						ag = (*i).second;
					}
				}
				else if (minDistance > (*i).second->distanceToEndOfSegment) {
					minDistance = (*i).second->distanceToEndOfSegment;
					agLane = (*i).first;
					ag = (*i).second;
				}
			}
		}
		frontalAgents[agLane] = laneAgentsMap[agLane]->next();
		return ag;
	}

	void AgentKeeper::resetFrontalAgents() {
		typedef std::vector<sim_mob::Agent*>::iterator agIt;
		for (std::map<const sim_mob::Lane*, sim_mob::LaneAgents*>::iterator i = laneAgentsMap.begin();
				i != laneAgentsMap.end(); i++) {
			(*i).second->resetIterator();
			frontalAgents[(*i).first] = (*i).second->next();
		}
	}

	sim_mob::Agent* LaneAgents::next() {
		sim_mob::Agent* ag = nullptr;
		if (laneAgentsIt != laneAgents.end()) {
			ag = *laneAgentsIt;
			laneAgentsIt++;
		}
		return ag;
	}

	sim_mob::Agent* AgentKeeper::getNext() {
		sim_mob::Agent* ag = nullptr;
		if (!allAgentsProcessed()) {
			ag = agentClosestToStopLine();
		}
		return ag;
	}

	int sim_mob::LaneAgents::getQueuingAgentsCount() {
		return queueCount;
	}

	void sim_mob::LaneAgents::addAgent(sim_mob::Agent* ag) {
		laneAgents.push_back(ag);
		if(ag->isQueuing) {
			queueCount++;
		}
	}

	void sim_mob::LaneAgents::removeAgent(sim_mob::Agent* ag) {
		std::vector<sim_mob::Agent*>::iterator agIt =  std::find(laneAgents.begin(), laneAgents.end(), ag);
		if(agIt != laneAgents.end()){
			laneAgents.erase(agIt);
		}
		if(ag->isQueuing) {
			queueCount--;
		}
	}

	sim_mob::Agent* sim_mob::LaneAgents::dequeue() {
		sim_mob::Agent* ag = laneAgents.front();
		laneAgents.erase(laneAgents.begin());
		if(ag->isQueuing) {
			queueCount--;
		}
		return ag;
	}

	int sim_mob::LaneAgents::getMovingAgentsCount() {
		return (laneAgents.size() - queueCount);
	}

	void LaneAgents::resetIterator() {
		laneAgentsIt = laneAgents.begin();
	}

}// end of namespace sim_mob

