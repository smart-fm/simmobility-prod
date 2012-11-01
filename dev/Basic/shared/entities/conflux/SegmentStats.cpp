/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <algorithm>
#include "SegmentStats.hpp"

using std::string;

namespace sim_mob {

	SegmentStats::SegmentStats(const sim_mob::RoadSegment* rdSeg, bool isDownstream)
		: roadSegment(rdSeg)
	{
		// initialize LaneAgents in the map
		std::vector<sim_mob::Lane*>::const_iterator lane = rdSeg->getLanes().begin();
		int num = rdSeg->getLanes().size();
		while(lane != rdSeg->getLanes().end())
		{
			laneStatsMap.insert(std::make_pair(*lane, new sim_mob::LaneStats()));
			double upSpeed = (*lane)->getRoadSegment()->maxSpeed/3.6 *100; //converting from kmph to m/s
			laneStatsMap[*lane]->initLaneParams(*lane, upSpeed);
			lane++;
		}
	}


	void SegmentStats::addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap[lane]->addAgent(ag);
	}

	void SegmentStats::removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap[lane]->removeAgent(ag);
	}

	sim_mob::Agent* SegmentStats::dequeue(const sim_mob::Lane* lane) {
		return laneStatsMap[lane]->dequeue();
	}

	std::vector<sim_mob::Agent*> SegmentStats::getAgents(const sim_mob::Lane* lane) {
		return laneStatsMap[lane]->laneAgents;
	}

	void SegmentStats::absorbAgents(sim_mob::SegmentStats* segStats)
	{
		if(roadSegment == segStats->getRoadSegment())
		{
			for(std::vector<sim_mob::Lane*>::const_iterator i = roadSegment->getLanes().begin();
					i != roadSegment->getLanes().end(); i++ ){
				laneStatsMap[*i]->addAgents(segStats->getAgents(*i), segStats->getLaneAgentCounts(*i).first);
			}
		}
	}

	std::pair<unsigned int, unsigned int> SegmentStats::getLaneAgentCounts(const sim_mob::Lane* lane) {
		return std::make_pair(
				laneStatsMap[lane]->getQueuingAgentsCount(),
				laneStatsMap[lane]->getMovingAgentsCount()
				);
	}

	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > SegmentStats::getAgentCountsOnLanes() {
		std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > agentCountsOnLanes;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			agentCountsOnLanes.insert(std::make_pair(*lane, getLaneAgentCounts(*lane)));
			lane++;
		}
		return agentCountsOnLanes;
	}

	const sim_mob::RoadSegment* sim_mob::SegmentStats::getRoadSegment() const {
		return roadSegment;
	}

	bool SegmentStats::isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent) {
		return (agent == laneStatsMap[lane]->laneAgents.front());
	}

	unsigned int SegmentStats::numAgentsInLane(const sim_mob::Lane* lane) {
		return (laneStatsMap[lane]->getMovingAgentsCount() + laneStatsMap[lane]->getQueuingAgentsCount());
	}

	unsigned int SegmentStats::numMovingInSegment(bool hasVehicle) {
		int movingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				movingCounts = movingCounts + laneStatsMap[*lane]->getMovingAgentsCount();
			}
			lane++;
		}
		return movingCounts;
	}

	unsigned int SegmentStats::numQueueingInSegment(bool hasVehicle) {
		int queuingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				queuingCounts = queuingCounts + laneStatsMap[*lane]->getQueuingAgentsCount();
			}
			lane++;
		}
		return queuingCounts;
	}

	bool SegmentStats::allAgentsProcessed() {
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

	sim_mob::Agent* SegmentStats::agentClosestToStopLine() {
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
		frontalAgents[agLane] = laneStatsMap[agLane]->next();
		return ag;
	}

	void SegmentStats::resetFrontalAgents() {
		typedef std::vector<sim_mob::Agent*>::iterator agIt;
		for (std::map<const sim_mob::Lane*, sim_mob::LaneStats*>::iterator i = laneStatsMap.begin();
				i != laneStatsMap.end(); i++) {
			(*i).second->resetIterator();
			frontalAgents[(*i).first] = (*i).second->next();
		}
	}

	std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > SegmentStats::getPrevTickLaneCountsFromOriginal() const {
		if(isDownstreamCopy()) {
			return prevTickLaneCountsFromOriginal;
		}
		return std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> >();
	}

	void SegmentStats::setPrevTickLaneCountsFromOriginal(std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginal) {
		if(isDownstreamCopy()) {
			this->prevTickLaneCountsFromOriginal = prevTickLaneCountsFromOriginal;
		}
	}

	sim_mob::Agent* SegmentStats::getNext() {
		sim_mob::Agent* ag = nullptr;
		if (!allAgentsProcessed()) {
			ag = agentClosestToStopLine();
		}
		return ag;
	}

	sim_mob::Agent* LaneStats::next() {
		sim_mob::Agent* ag = nullptr;
		if (laneAgentsIt != laneAgents.end()) {
			ag = *laneAgentsIt;
			laneAgentsIt++;
		}
		return ag;
	}

	int sim_mob::LaneStats::getQueuingAgentsCount() {
		return queueCount;
	}

	void sim_mob::LaneStats::addAgent(sim_mob::Agent* ag) {
		laneAgents.push_back(ag);
		if(ag->isQueuing) {
			queueCount++;
		}
	}

	void LaneStats::addAgents(std::vector<sim_mob::Agent*> agents, unsigned int numQueuing) {
		queueCount = queueCount + numQueuing;
		laneAgents.insert(laneAgents.end(), agents.begin(), agents.end());
	}

	void sim_mob::LaneStats::removeAgent(sim_mob::Agent* ag) {
		std::vector<sim_mob::Agent*>::iterator agIt =  std::find(laneAgents.begin(), laneAgents.end(), ag);
		if(agIt != laneAgents.end()){
			laneAgents.erase(agIt);
		}
		if(ag->isQueuing) {
			queueCount--;
		}
	}

	sim_mob::Agent* sim_mob::LaneStats::dequeue() {
		sim_mob::Agent* ag = laneAgents.front();
		laneAgents.erase(laneAgents.begin());
		if(ag->isQueuing) {
			queueCount--;
		}
		return ag;
	}

	int sim_mob::LaneStats::getMovingAgentsCount() {
		return (laneAgents.size() - queueCount);
	}

	void LaneStats::resetIterator() {
		laneAgentsIt = laneAgents.begin();
	}

	void sim_mob::LaneStats::initLaneParams(const Lane* lane, double upSpeed) {
		//laneParams = sim_mob::LaneParams();
		int numLanes = lane->getRoadSegment()->getLanes().size();
		if (numLanes > 0) {
			double orig = lane->getRoadSegment()->capacity/(numLanes*3600.0);
			laneParams->setOrigOutputFlowRate(orig);
		}
		laneParams->outputFlowRate = laneParams->origOutputFlowRate;

		updateOutputCounter(lane);
		updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::LaneStats::updateOutputFlowRate(const Lane* lane, double newFlowRate) {
		laneParams->outputFlowRate = newFlowRate;
	}

	void sim_mob::LaneStats::updateOutputCounter(const Lane* lane) {
		double elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;
		int tmp = int(laneParams->outputFlowRate*elapsedSeconds);
		laneParams->fraction += laneParams->outputFlowRate*elapsedSeconds - float(tmp);
		if (laneParams->fraction >= 1.0) {
			laneParams->fraction -= 1.0;
			laneParams->outputCounter = float(tmp) + 1.0;
		} else
			laneParams->outputCounter = float(tmp);
		std::stringstream ss;
		ss << "Lane: " << lane->getLaneID_str()
		<< "\toutputFlowRate: "<< laneParams->outputFlowRate
		<< "\toutputCounter: " << laneParams->outputCounter
		<< "\tfraction: " << laneParams->fraction<< std::endl;
		std::cout << ss.str();
	}

	void sim_mob::LaneStats::updateAcceptRate(const Lane* lane, double upSpeed) {
		const double omega = 0.01;
		const double vehicle_length = 400;
		double elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;
		double capacity = laneParams->outputFlowRate*elapsedSeconds;
		double acceptRateA = (capacity > 0) ? elapsedSeconds / capacity : 0;
		double acceptRateB = (omega*vehicle_length)/upSpeed;
		double acceptRate = std::max( acceptRateA, acceptRateB);
	}

	sim_mob::LaneParams* sim_mob::SegmentStats::getLaneParams(const Lane* lane) {
		return laneStatsMap[lane]->laneParams;
	}

	double sim_mob::SegmentStats::speed_density_function(bool hasVehicle) {
	//	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	//	if ( !rf.isKnownRole(roleName))
		unsigned int numVehicles = numMovingInSegment(hasVehicle);
		/**
		 * TODO: The parameters - min density, jam density, alpha and beta - for each road segment
		 * must be obtained from an external source (XML/Database)
		 * Since we don't have this data, we have taken the average values from supply parameters of Singapore express ways.
		 * This must be changed after we have this data for each road segment in the network.
		 *
		 * TODO: A params struct for these parameters is already defined in the RoadSegment class.
		 * This struct is to be used when we have actual values for the parameters.
		 */

		double density = numVehicles / (getRoadSegment()->computeLaneZeroLength() / 100.0);

		double freeFlowSpeed = getRoadSegment()->maxSpeed / 3.6 * 100; // Converting from Kmph to cm/s
		double minSpeed = 0.0;
		double jamDensity = 1; //density during traffic jam
		double alpha = 3.75; //Model parameter of speed density function
		double beta = 0.5645; //Model parameter of speed density function
		double minDensity = 0.0048; // minimum traffic density

		//Speed-Density function
		if(density <= minDensity){
			return freeFlowSpeed;
		}
		else if (density >= jamDensity) {
			return minSpeed;
		}
		else {
			//TODO: Remove debugging print statement later ~ Harish
			//ss << "!! " << "density:" << density << "!! " << freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, beta)),alpha) << " !!" << std::endl;
			return freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, beta)),alpha);
		}
	}

	void sim_mob::SegmentStats::restoreLaneParams(const Lane* lane){
		laneStatsMap[lane]->updateOutputFlowRate(lane, getLaneParams(lane)->origOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		double upSpeed = speed_density_function(true);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate){
		laneStatsMap[lane]->updateOutputFlowRate(lane, newOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		double upSpeed = speed_density_function(true);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}
}// end of namespace sim_mob

