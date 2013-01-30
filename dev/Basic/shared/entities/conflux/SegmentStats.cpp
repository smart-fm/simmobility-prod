/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <algorithm>
#include "SegmentStats.hpp"
#include "util/OutputUtil.hpp"

using std::string;

namespace sim_mob {

	SegmentStats::SegmentStats(const sim_mob::RoadSegment* rdSeg, bool isDownstream)
		: roadSegment(rdSeg), segDensity(0.0), lastAcceptTime(0.0), debugMsgs(std::stringstream::out)
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

		laneInfinity = new sim_mob::Lane();
		laneStatsMap.insert(std::make_pair(laneInfinity, new sim_mob::LaneStats()));

		downstreamCopy = isDownstream;
	}


	void SegmentStats::addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		if(laneStatsMap.find(lane) != laneStatsMap.end()) {
			laneStatsMap[lane]->addAgent(ag);
		}
		else{
			throw std::runtime_error("SegmentStats::addAgent called with invalid laneStats.");
		}
	}

	void SegmentStats::removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		if(laneStatsMap.find(lane) != laneStatsMap.end()) {
			laneStatsMap[lane]->removeAgent(ag);
		}
		else{
			throw std::runtime_error("SegmentStats::removeAgent called with invalid laneStats.");
		}
	}

	void SegmentStats::updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		if(laneStatsMap.find(lane) != laneStatsMap.end()) {
			laneStatsMap[lane]->updateQueueStatus(ag);
		}
		else{
			throw std::runtime_error("SegmentStats::updateQueueStatus called with invalid laneStats.");
		}
	}

	sim_mob::Agent* SegmentStats::dequeue(const sim_mob::Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::dequeue called with invalid laneStats.");
		}
		return laneStatsMap[lane]->dequeue();
	}

	std::vector<sim_mob::Agent*> SegmentStats::getAgents(const sim_mob::Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
				throw std::runtime_error("SegmentStats::getAgents called with invalid laneStats.");
		}
		return laneStatsMap[lane]->laneAgents;
	}

	void SegmentStats::absorbAgents(sim_mob::SegmentStats* segStats)
	{
		if(roadSegment == segStats->getRoadSegment())
		{
			for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator i = laneStatsMap.begin();
					i != laneStatsMap.end(); i++ ){
				if(i->first == laneInfinity) { // Lane infinities are created individually for each segment stats
					std::vector<sim_mob::Agent*> agentsOnLnInfinity = segStats->getAgents(segStats->laneInfinity);
					std::vector<sim_mob::Agent*>::iterator agIt = agentsOnLnInfinity.begin();
					while (agIt != agentsOnLnInfinity.end()) {
						(*agIt)->setCurrLane(i->first);
						i->second->addAgent(*agIt);
						agIt++;
					}
				}
				else {
					i->second->addAgents(segStats->getAgents(i->first), segStats->getLaneAgentCounts(i->first).first);
				}
			}
		}
		else {
			throw std::runtime_error("SegmentStats::absorbAgents(segStats) called with invalid segStats.");
		}
	}

	std::pair<unsigned int, unsigned int> SegmentStats::getLaneAgentCounts(const sim_mob::Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::getLaneAgentCounts called with invalid laneStats.");
		}
		return std::make_pair(
				laneStatsMap[lane]->getQueuingAgentsCount(),
				laneStatsMap[lane]->getMovingAgentsCount()
				);
	}

	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > SegmentStats::getAgentCountsOnLanes() {
		std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > agentCountsOnLanes;
		std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator laneStatsIt = laneStatsMap.begin();
		while(laneStatsIt != laneStatsMap.end())
		{
			agentCountsOnLanes.insert(std::make_pair(laneStatsIt->first, getLaneAgentCounts(laneStatsIt->first)));
			laneStatsIt++;
		}
		return agentCountsOnLanes;
	}

	const sim_mob::RoadSegment* sim_mob::SegmentStats::getRoadSegment() const {
		return roadSegment;
	}

	bool SegmentStats::isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::isFront called with invalid laneStats.");
		}
		return (agent == laneStatsMap[lane]->laneAgents.front());
	}

	unsigned int SegmentStats::numAgentsInLane(const sim_mob::Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::numAgentsInLane called with invalid laneStats.");
		}
		return (laneStatsMap[lane]->getMovingAgentsCount() + laneStatsMap[lane]->getQueuingAgentsCount());
	}

	unsigned int SegmentStats::numMovingInSegment(bool hasVehicle) {
		unsigned int movingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();

		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				if(laneStatsMap.find(*lane) != laneStatsMap.end()) {
					if(laneStatsMap[*lane]->getMovingAgentsCount() > 1000){
						debugMsgs<< "large moving count: "<<roadSegment->getStart()->getID() <<" lane: "
						<<"queueCount: "<<laneStatsMap[*lane]->getQueuingAgentsCount()
						<< (*lane)->getLaneID_str()<<std::endl;
						std::cout << debugMsgs.str();
						debugMsgs.str("");
						throw std::runtime_error("SegmentStats::numMovingInSegment called with invalid laneStats.");
					}
					movingCounts = movingCounts + laneStatsMap[*lane]->getMovingAgentsCount();
				}
			}
			lane++;
		}

		return movingCounts;
	}

	double SegmentStats::getDensity(bool hasVehicle) {
		int vehLaneCount = 0;
		double movingLength = 0.0;
		const int vehicle_length = 400;
		double density = 0.0;

		std::vector<sim_mob::Lane*>::const_iterator laneIt = roadSegment->getLanes().begin();
		while(laneIt != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*laneIt)->is_pedestrian_lane())
				|| ( !hasVehicle && (*laneIt)->is_pedestrian_lane()))
			{
				vehLaneCount += 1;
			}
			laneIt++;
		}
		movingLength = roadSegment->computeLaneZeroLength()*vehLaneCount
								- numQueueingInSegment(true)*vehicle_length;
		if(movingLength > 0)
		{
			density = numMovingInSegment(true)/(movingLength/100.0);
		}
		else
			density = 1/(vehicle_length/100.0);

		return density;
	}

	unsigned int SegmentStats::numQueueingInSegment(bool hasVehicle) {
		unsigned int queuingCounts = 0;

		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				if(laneStatsMap.find(*lane) != laneStatsMap.end()) {
					queuingCounts = queuingCounts + laneStatsMap[*lane]->getQueuingAgentsCount();
				}
				else{
					throw std::runtime_error("SegmentStats::numQueueingInSegment called with invalid laneStats.");
				}
			}
			lane++;
		}
		return queuingCounts;
	}

	bool SegmentStats::allAgentsProcessed() {
		bool allAgentsProcessed = true;
		std::map<const sim_mob::Lane*, sim_mob::Agent* >::iterator i = frontalAgents.begin();
		while(i!=frontalAgents.end()) {
			if(i->second) {
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
			if(i->second) {
				if(minDistance == i->second->distanceToEndOfSegment) {
					// If current ag and (*i) are at equal distance to the stop line, we toss a coin and choose one of them
					bool coinTossResult = ((rand() / (double)RAND_MAX) < 0.5);
					if(coinTossResult) {
						agLane = i->first;
						ag = i->second;
					}
				}
				else if (minDistance > i->second->distanceToEndOfSegment) {
					minDistance = i->second->distanceToEndOfSegment;
					agLane = i->first;
					ag = i->second;
				}
			}
			i++;
		}
		frontalAgents.erase(agLane);
		if(laneStatsMap.find(agLane) != laneStatsMap.end()){
			frontalAgents[agLane] = laneStatsMap[agLane]->next();
		}
		else{
			throw std::runtime_error("SegmentStats::agentClosestToStopLine called with invalid laneStats.");
		}
		return ag;
	}

	void SegmentStats::resetFrontalAgents() {
		frontalAgents.clear();
		for (std::map<const sim_mob::Lane*, sim_mob::LaneStats*>::iterator i = laneStatsMap.begin();
				i != laneStatsMap.end(); i++) {
			i->second->resetIterator();
			Agent* agent = i->second->next();
			frontalAgents.insert(std::make_pair(i->first, agent));
		}
	}

	std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > SegmentStats::getPrevTickLaneCountsFromOriginal() const {
		if(isDownstreamCopy()) {
			return prevTickLaneCountsFromOriginal;
		}
		return std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> >();
	}

	void SegmentStats::setPrevTickLaneCountsFromOriginal(std::map<const sim_mob::Lane*, std::pair<unsigned int, unsigned int> > prevTickLaneCountsFromOriginalCopy) {
		if(isDownstreamCopy()) {
			prevTickLaneCountsFromOriginal = prevTickLaneCountsFromOriginalCopy;
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
		if (laneAgentsIt != laneAgentsCopy.end()) {
			ag = *laneAgentsIt;
			laneAgentsIt++;
		}
		return ag;
	}

	unsigned int sim_mob::LaneStats::getQueuingAgentsCount() {
		return queueCount;
	}

	unsigned int sim_mob::LaneStats::getMovingAgentsCount() {
		if(laneAgents.size() < queueCount){
			std::cout<<"queueCount: "<<queueCount << " |laneAgents count: "<<laneAgents.size()
					<<"moving: "<< laneAgents.size()-queueCount<<std::endl;
		//	throw std::runtime_error("number of lane agents cannot be less than the number of "
		//			"queuing agents");
		}
		return (laneAgents.size() - queueCount);
	}

	void sim_mob::LaneStats::addAgent(sim_mob::Agent* ag) {
		if (std::find(laneAgents.begin(), laneAgents.end(), ag)==laneAgents.end()) // if agent is not already in the lane add him
			laneAgents.push_back(ag);
		else
			throw std::runtime_error("Attempting to addAgent to the lane twice!");
		if(ag->isQueuing)
			queueCount++;
	}

	void sim_mob::LaneStats::updateQueueStatus(sim_mob::Agent* ag) {
		if (std::find(laneAgents.begin(), laneAgents.end(), ag)==laneAgents.end()) // if agent is not already in the lane add him
			throw std::runtime_error("Attempting to queue an agent who's not in lane!");

		if(ag->isQueuing)
			queueCount++;
		else
			queueCount--;
	}

	void LaneStats::addAgents(std::vector<sim_mob::Agent*> agents, unsigned int numQueuing) {
		if(agents.size() > 0) {
			queueCount = queueCount + numQueuing;
			laneAgents.insert(laneAgents.end(), agents.begin(), agents.end());
		}
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
		laneAgentsCopy = laneAgents;
		laneAgentsIt = laneAgentsCopy.begin();
	}

	void sim_mob::LaneStats::initLaneParams(const Lane* lane, double vehSpeed, double pedSpeed) {
		//laneParams = sim_mob::LaneParams();
		int numLanes = lane->getRoadSegment()->getLanes().size();
		if (numLanes > 0) {
			double orig = (lane->getRoadSegment()->capacity)/(numLanes/**3600.0*/);
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
	}

	void sim_mob::LaneStats::updateAcceptRate(const Lane* lane, double upSpeed) {
		const double omega = 0.01;
		const double vehicle_length = 400;
		double elapsedSeconds = ConfigParams::GetInstance().baseGranMS / 1000.0;
		double capacity = laneParams->outputFlowRate*elapsedSeconds;
		double acceptRateA = (capacity > 0) ? elapsedSeconds / capacity : 0;
		double acceptRateB = (omega*vehicle_length)/upSpeed;
		laneParams->acceptRate = std::max( acceptRateA, acceptRateB);
	}

	void LaneStats::clear() {
		laneAgents.clear();
	}

	sim_mob::LaneParams* sim_mob::SegmentStats::getLaneParams(const Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::getLaneParams called with invalid laneStats.");
		}
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
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::restoreLaneParams called with invalid laneStats.");
		}
		laneStatsMap[lane]->updateOutputFlowRate(lane, getLaneParams(lane)->origOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate){
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::updateLaneParams called with invalid laneStats.");
		}
		laneStatsMap[lane]->updateOutputFlowRate(lane, newOutputFlowRate);
		laneStatsMap[lane]->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStatsMap[lane]->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(timeslice frameNumber){
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
				std::stringstream ss;
				(it->second)->setInitialQueueCount(it->second->getQueuingAgentsCount());
			}
		}
	}

	void sim_mob::SegmentStats::reportSegmentStats(timeslice frameNumber){
#ifndef SIMMOB_DISABLE_OUTPUT
//		("segmentState",20,0xa0e30d8,{"speed":"10.4","flow":"8","density":"12"})
		LogOut("(\"segmentState\""
			<<","<<frameNumber.frame()
			<<","<<roadSegment
			//<<","<<roadSegment->getStart()->getID()
			//<<","<<roadSegment->getEnd()->getID()
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

	bool SegmentStats::hasAgents() {
		std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator laneIt = laneStatsMap.begin();
		while(laneIt != laneStatsMap.end())
		{
			if(laneIt->second->laneAgents.size() > 0){
				return true;
			}
			laneIt++;
		}
		return false;
	}

	double SegmentStats::getPositionOfLastUpdatedAgentInLane(const Lane* lane) {
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::getPositionOfLastUpdatedAgentInLane"
					" called with invalid laneStats.");
		}
		return laneStatsMap[lane]->getPositionOfLastUpdatedAgent();
	}

	void SegmentStats::setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane) {
		if(lane != laneInfinity and laneStatsMap.find(lane) != laneStatsMap.end()) {
			laneStatsMap[lane]->setPositionOfLastUpdatedAgent(positionOfLastUpdatedAgentInLane);
		}
	}

	unsigned int sim_mob::SegmentStats::getInitialQueueCount(const Lane* lane){
		if(laneStatsMap.find(lane) == laneStatsMap.end()) {
			throw std::runtime_error("SegmentStats::getInitialQueueCount"
								" called with invalid laneStats.");
		}
		return laneStatsMap[lane]->getInitialQueueCount();
	}

	void SegmentStats::clear() {
		// Only agents in the downstream copy of SegmentStats are meant to cleared with this function.
		if(isDownstreamCopy())
		{
			for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator i = laneStatsMap.begin();
					i != laneStatsMap.end(); i++)
			{
				i->second->clear();
			}
		}

	}

	void SegmentStats::resetPositionOfLastUpdatedAgentOnLanes(){
		for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator i = laneStatsMap.begin();
							i != laneStatsMap.end(); i++)
		{
			i->second->setPositionOfLastUpdatedAgent(-1.0);
		}
	}
}// end of namespace sim_mob

