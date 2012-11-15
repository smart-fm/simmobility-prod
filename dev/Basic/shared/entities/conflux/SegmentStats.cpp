/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <algorithm>
#include "SegmentStats.hpp"
#include "util/OutputUtil.hpp"

using std::string;

namespace sim_mob {

	SegmentStats::SegmentStats(const sim_mob::RoadSegment* rdSeg, bool isDownstream)
		: roadSegment(rdSeg), segDensity(0.0)
	{
		segVehicleSpeed = getRoadSegment()->maxSpeed/3.6 *100; //converting from kmph to m/s
		segPedSpeed = 0.0;
		// initialize LaneAgents in the map
		std::vector<sim_mob::Lane*>::const_iterator lane = rdSeg->getLanes().begin();
		while(lane != rdSeg->getLanes().end())
		{
			laneStatsMap.insert(std::make_pair(*lane, new sim_mob::LaneStats()));
			laneStatsMap[*lane]->initLaneParams(*lane, segVehicleSpeed, segPedSpeed);
			lane++;
		}
	}


	void SegmentStats::addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap[lane]->addAgent(ag);
	}

	void SegmentStats::removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap[lane]->removeAgent(ag);
	}

	void SegmentStats::dequeue(const sim_mob::Lane* lane) {
		if(lane) {
			laneStatsMap[lane]->dequeue();
		}
		else {
			// no lane indicates laneInfinity
			laneInfinity.pop();
		}
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
		else {
			throw std::runtime_error("SegmentStats::absorbAgents(segStats) called with invalid segStats.");
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
		std::vector<sim_mob::Lane*>::const_iterator laneIt = roadSegment->getLanes().begin();
		while(laneIt != roadSegment->getLanes().end())
		{
			agentCountsOnLanes.insert(std::make_pair(*laneIt, getLaneAgentCounts(*laneIt)));
			laneIt++;
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

	double SegmentStats::getDensity(bool hasVehicle) {
		int movingCounts = 0;
		double movingLength = 0.0;
		const int vehicle_length = 400;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				movingCounts += laneStatsMap[*lane]->getMovingAgentsCount();
				movingLength += roadSegment->computeLaneZeroLength()
						- laneStatsMap[*lane]->getQueuingAgentsCount()*vehicle_length;
			}
			lane++;
		}
		return movingCounts/(movingLength/100.0);
	}

	unsigned int SegmentStats::numQueueingInSegment(bool hasVehicle) {
		int queuingCounts = 0;
		std::stringstream ss;
		if(roadSegment)
				ss << "RoadSegment: "<< roadSegment
						/*<<"\tnLanes: "<< roadSegment->getLanes().size()*/ << std::endl;
				std::cout << ss.str();
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
			i++;
		}
		return allAgentsProcessed;
	}

	sim_mob::Agent* SegmentStats::agentClosestToStopLine() {
		sim_mob::Agent* ag = nullptr;
		const sim_mob::Lane* agLane = nullptr;
		double minDistance = std::numeric_limits<double>::max();

		std::map<const sim_mob::Lane*, sim_mob::Agent* >::iterator i = frontalAgents.begin();
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

	void SegmentStats::setPrevTickLaneCountsFromOriginal(std::map<sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginalCopy) {
		if(isDownstreamCopy()) {
			prevTickLaneCountsFromOriginal = prevTickLaneCountsFromOriginalCopy;
		}
	}

	sim_mob::Agent* SegmentStats::getNext() {
		sim_mob::Agent* ag = nullptr;
		if (!allAgentsProcessed()) {
			ag = agentClosestToStopLine();
		}
		else {
			/* If all agents which were already in the SegmentStats are processed,
			 * we must process the new starting agents in laneInfinity
			 */
			if (laneInfinity.size() > 0) {
				ag = laneInfinity.top();
				laneInfinity.pop();
			}
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

	unsigned int sim_mob::LaneStats::getQueuingAgentsCount() {
		return queueCount;
	}

	unsigned int sim_mob::LaneStats::getMovingAgentsCount() {
		return (laneAgents.size() - queueCount);
	}

	void sim_mob::LaneStats::addAgent(sim_mob::Agent* ag) {
		if (std::find(laneAgents.begin(), laneAgents.end(), ag)!=laneAgents.end())
			laneAgents.push_back(ag);
		else
			throw std::runtime_error("addAgentToQueue for agent who's not added to the lane");
		if(ag->isQueuing)
			queueCount++;
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

	void LaneStats::resetIterator() {
		laneAgentsIt = laneAgents.begin();
	}

	void sim_mob::LaneStats::initLaneParams(const Lane* lane, double vehSpeed, double pedSpeed) {
		//laneParams = sim_mob::LaneParams();
		int numLanes = lane->getRoadSegment()->getLanes().size();
		if (numLanes > 0) {
			double orig = lane->getRoadSegment()->capacity/(numLanes*3600.0);
			laneParams->setOrigOutputFlowRate(orig);
		}
		laneParams->outputFlowRate = laneParams->origOutputFlowRate;

		updateOutputCounter(lane);
		updateAcceptRate(lane, vehSpeed);
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

	double sim_mob::SegmentStats::speed_density_function(bool hasVehicle, double segDensity) {
	//	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	//	if ( !rf.isKnownRole(roleName))
	//	unsigned int numVehicles = numMovingInSegment(hasVehicle);
	//	double segDensity = getDensity(hasVehicle); // computes density using only moving length
		/**
		 * TODO: The parameters - min density, jam density, alpha and beta - for each road segment
		 * must be obtained from an external source (XML/Database)
		 * Since we don't have this data, we have taken the average values from supply parameters of Singapore express ways.
		 * This must be changed after we have this data for each road segment in the network.
		 *
		 * TODO: A params struct for these parameters is already defined in the RoadSegment class.
		 * This struct is to be used when we have actual values for the parameters.
		 */

		//double density = numVehicles / (getRoadSegment()->computeLaneZeroLength() / 100.0);

		double freeFlowSpeed = getRoadSegment()->maxSpeed / 3.6 * 100; // Converting from Kmph to cm/s
		double minSpeed = 0.0;
		double jamDensity = 1; //density during traffic jam
		double alpha = 3.75; //Model parameter of speed density function
		double beta = 0.5645; //Model parameter of speed density function
		double minDensity = 0.0048; // minimum traffic density

		//Speed-Density function
		if(segDensity <= minDensity){
			return freeFlowSpeed;
		}
		else if (segDensity >= jamDensity) {
			return minSpeed;
		}
		else {
			//TODO: Remove debugging print statement later ~ Harish
			//ss << "!! " << "density:" << density << "!! " << freeFlowSpeed * pow((1 - pow((density - minDensity)/jamDensity, beta)),alpha) << " !!" << std::endl;
			return freeFlowSpeed * pow((1 - pow((segDensity - minDensity)/jamDensity, beta)),alpha);
		}
	}

	void sim_mob::SegmentStats::restoreLaneParams(const Lane* lane){
		laneStatsMap[lane]->updateOutputFlowRate(lane, getLaneParams(lane)->origOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate){
		laneStatsMap[lane]->updateOutputFlowRate(lane, newOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(frame_t frameNumber){
		segDensity = getDensity(true);
		segVehicleSpeed = speed_density_function(true, segDensity);
		//need to update segPedSpeed in future
		std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator it = laneStatsMap.begin();
		for( ; it != laneStatsMap.end(); ++it )
		{
			//filtering out the pedestrian lanes for now
			if ( !(it->first)->is_pedestrian_lane()){
				(it->second)->updateOutputCounter(it->first);
				(it->second)->updateAcceptRate(it->first, segVehicleSpeed);
				(it->second)->setInitialQueueCount(it->second->getQueuingAgentsCount());
			}
		}
	}

	void sim_mob::SegmentStats::reportSegmentStats(frame_t frameNumber){
#ifndef SIMMOB_DISABLE_OUTPUT
//		("segmentState",20,0xa0e30d8,{"speed":"10.4","flow":"8","density":"12"})
		LogOut("(\"segmentState\""
			<<","<<frameNumber
			<<","<<roadSegment->getId()
			<<",{"
			<<"\"speed\":\""<<segVehicleSpeed
			<<"\",\"flow\":\""<<0
			<<"\",\"density\":\""<<segDensity
			<<"\"})"<<std::endl);
#endif

	}

	double sim_mob::SegmentStats::getSegSpeed(bool hasVehicle){
		if (hasVehicle) return segVehicleSpeed;
		else return segPedSpeed;
	}

	unsigned int sim_mob::SegmentStats::getInitialQueueCount(const Lane* lane){
		return laneStatsMap[lane]->getInitialQueueCount();
	}
}// end of namespace sim_mob

