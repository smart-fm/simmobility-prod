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
			laneStatsMap.insert(std::make_pair(*lane, new sim_mob::LaneStats(*lane)));
			laneStatsMap[*lane]->initLaneParams(*lane, segVehicleSpeed, segPedSpeed);
			prevTickLaneCountsFromOriginal.insert(std::make_pair(*lane, std::make_pair(0,0))); // initialized to zero (irrespective of whether this is downstreamCopy)
			lane++;
		}

		/*
		 * Any lane with an id ending with 9 is laneInfinity of the road segment.
		 * TODO: Must check if we can have a bit pattern (Refer lane constructor) for laneInfinity.
		 */
		laneInfinity = new sim_mob::Lane(const_cast<sim_mob::RoadSegment*>(rdSeg), 9);
		laneStatsMap.insert(std::make_pair(laneInfinity, new sim_mob::LaneStats(laneInfinity)));
		prevTickLaneCountsFromOriginal.insert(std::make_pair(laneInfinity, std::make_pair(0,0)));
		downstreamCopy = isDownstream;
	}


	void SegmentStats::addAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap.find(lane)->second->addAgent(ag);
		if(lane != laneInfinity) laneStatsMap.find(lane)->second->verifyOrdering();
	}

	void SegmentStats::removeAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap.find(lane)->second->removeAgent(ag);
	}

	void SegmentStats::updateQueueStatus(const sim_mob::Lane* lane, sim_mob::Agent* ag) {
		laneStatsMap.find(lane)->second->updateQueueStatus(ag);
	}

	sim_mob::Agent* SegmentStats::dequeue(const sim_mob::Lane* lane) {
		return laneStatsMap.find(lane)->second->dequeue();
	}

	std::vector<sim_mob::Agent*> SegmentStats::getAgents(const sim_mob::Lane* lane) {
		return laneStatsMap.find(lane)->second->laneAgents;
	}

	void SegmentStats::absorbAgents(sim_mob::SegmentStats* segStats)
	{
		if(roadSegment == segStats->getRoadSegment())
		{
			for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++ ){
				if(i->first == laneInfinity) { // Lane infinities are created individually for each segment stats
					std::vector<sim_mob::Agent*> agentsOnLnInfinity = segStats->getAgents(segStats->laneInfinity);
					std::vector<sim_mob::Agent*>::iterator agIt = agentsOnLnInfinity.begin();
					while (agIt != agentsOnLnInfinity.end()) {
						(*agIt)->setCurrLane(i->first); // lane infinities are different for each SegmentStats. It has to be set explicitly.
						i->second->addAgent(*agIt);
						agIt++;
					}
				}
				else {
					i->second->addAgents(segStats->getAgents(i->first), segStats->getLaneAgentCounts(i->first).first /*Queuing Count*/);
				}
			}
		}
		else {
			throw std::runtime_error("SegmentStats::absorbAgents(segStats) called with invalid segStats.");
		}
	}

	std::pair<unsigned int, unsigned int> SegmentStats::getLaneAgentCounts(const sim_mob::Lane* lane) {
		if(isDownstreamCopy()) {
			return std::make_pair(
				laneStatsMap.at(lane)->getQueuingAgentsCount() + getPrevTickLaneCountsFromOriginal().at(lane).first,
				laneStatsMap.at(lane)->getMovingAgentsCount() + getPrevTickLaneCountsFromOriginal().at(lane).second
			);
		}
		return std::make_pair(
				laneStatsMap.at(lane)->getQueuingAgentsCount(),
				laneStatsMap.at(lane)->getMovingAgentsCount()
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

	const sim_mob::RoadSegment* sim_mob::SegmentStats::getRoadSegment() const { return roadSegment; }

	bool SegmentStats::isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent) {
		return (agent == laneStatsMap.find(lane)->second->laneAgents.front());
	}

	unsigned int SegmentStats::numAgentsInLane(const sim_mob::Lane* lane) {
		if(isDownstreamCopy()) {
			return (laneStatsMap.at(lane)->getMovingAgentsCount()
					+ laneStatsMap.at(lane)->getQueuingAgentsCount()
					+ prevTickLaneCountsFromOriginal.at(lane).first
					+ prevTickLaneCountsFromOriginal.at(lane).second
					);
		}
		return (laneStatsMap.at(lane)->getMovingAgentsCount()
				+ laneStatsMap.at(lane)->getQueuingAgentsCount()
				);
	}

	unsigned int SegmentStats::numMovingInSegment(bool hasVehicle) {
		unsigned int movingCounts = 0;
		std::vector<sim_mob::Lane*>::const_iterator lane = roadSegment->getLanes().begin();
		while(lane != roadSegment->getLanes().end())
		{
			if ((hasVehicle && !(*lane)->is_pedestrian_lane())
				|| ( !hasVehicle && (*lane)->is_pedestrian_lane()))
			{
				std::map<const sim_mob::Lane*, sim_mob::LaneStats*>::iterator laneStatsIt = laneStatsMap.find(*lane);
				if(laneStatsIt != laneStatsMap.end()) {
					if(isDownstreamCopy()) {
						movingCounts = movingCounts + laneStatsIt->second->getMovingAgentsCount() + prevTickLaneCountsFromOriginal.at(laneStatsIt->first).second;
					}
					else {
						movingCounts = movingCounts + laneStatsIt->second->getMovingAgentsCount();
					}
				}
				else {
					throw std::runtime_error("SegmentStats::numMovingInSegment called with invalid laneStats.");
				}
			}
			lane++;
		}
		if(movingCounts > roadSegment->getLanes().size()*roadSegment->computeLaneZeroLength()/400.0){
			std::cout<<"large moving count "<< roadSegment->getStart()->getID()<<std::endl;
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
			if(density > 0.25) {
				debugMsgs<<"density problem | segment: ["<< roadSegment->getStart()->getID() << "," << roadSegment->getEnd()->getID() << "]"
						<< "| numMovingInSeg: "<< numMovingInSegment(true)
						<< "| numQueueingInSeg: " << numQueueingInSegment(true)
						<< "| numLanes: " << vehLaneCount
						<< "| movLength: "<< movingLength/100.0
						<< "| rdSegLength: "<< roadSegment->computeLaneZeroLength()
						<< "| density " << density
						<<std::endl;
				std::cout<<debugMsgs.str();
				debugMsgs.str("");
				throw std::runtime_error("error in segment Density");
			}
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
				std::map<const sim_mob::Lane*, sim_mob::LaneStats*>::iterator laneStatsIt = laneStatsMap.find(*lane);
				if(laneStatsIt != laneStatsMap.end()) {
					if(isDownstreamCopy()) {
						queuingCounts = queuingCounts + laneStatsIt->second->getQueuingAgentsCount() + prevTickLaneCountsFromOriginal.at(laneStatsIt->first).first;
					}
					else {
						queuingCounts = queuingCounts + laneStatsIt->second->getQueuingAgentsCount();
					}
				}
				else {
					throw std::runtime_error("SegmentStats::numQueueingInSegment was called with invalid laneStats!");
				}
			}
			lane++;
		}
		return queuingCounts;
	}

	sim_mob::Agent* SegmentStats::agentClosestToStopLineFromFrontalAgents() {
		sim_mob::Agent* ag = nullptr;
		const sim_mob::Lane* agLane = nullptr;
		double minDistance = std::numeric_limits<double>::max();

		std::map<const sim_mob::Lane*, sim_mob::Agent* >::iterator i = frontalAgents.begin();
		while(i!=frontalAgents.end()) {
			if(i->second) {
				if(minDistance == i->second->distanceToEndOfSegment) {
					// If current ag and (*i) are at equal distance to the stop line, we 'toss a coin' and choose one of them
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

		if(ag) { // frontalAgents could possibly be all nullptrs
			frontalAgents.erase(agLane);
			frontalAgents.insert(std::make_pair(agLane,laneStatsMap.at(agLane)->next()));
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
		laneAgents.push_back(ag);
		if (ag->isQueuing) queueCount++;
	}

	void sim_mob::LaneStats::updateQueueStatus(sim_mob::Agent* ag) {
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
		if(laneAgents.size() == 0){
			throw std::runtime_error("Trying to dequeue from empty lane.");
		}
		laneAgents.erase(laneAgents.begin());
		if(queueCount > 0) {
			// we have removed a queuing agent
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
		return laneStatsMap.find(lane)->second->laneParams;
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
		LaneStats* laneStats = laneStatsMap.find(lane)->second;
		laneStats->updateOutputFlowRate(lane, getLaneParams(lane)->origOutputFlowRate);
		laneStats->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStats->updateAcceptRate(lane, upSpeed);
	}

	void sim_mob::SegmentStats::updateLaneParams(const Lane* lane, double newOutputFlowRate){
		LaneStats* laneStats = laneStatsMap.find(lane)->second;
		laneStats->updateOutputFlowRate(lane, newOutputFlowRate);
		laneStats->updateOutputCounter(lane);
		segDensity = getDensity(true);
		double upSpeed = speed_density_function(true, segDensity);
		laneStats->updateAcceptRate(lane, upSpeed);
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
			<<",{"
			<<"\"speed\":\""<<segVehicleSpeed
			<<"\",\"flow\":\""<<0
			<<"\",\"density\":\""<<segDensity
			<<"\"})"<<std::endl);
#endif
		debugMsgs << "(\"segmentState\""
					<<","<<frameNumber.frame()
					<<","<<roadSegment->getStart()->getID()
					<<",{"
					<<"\"speed\":\""<<segVehicleSpeed
					<<"\",\"flow\":\""<<0
					<<"\",\"density\":\""<<segDensity
					<<"\"})"<<std::endl;
		std::cout<<debugMsgs.str();
		debugMsgs.str("");
	}

	double sim_mob::SegmentStats::getSegSpeed(bool hasVehicle){
		if (hasVehicle) return segVehicleSpeed;
		else return segPedSpeed;
	}

	bool SegmentStats::hasAgents() {
		std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::iterator laneIt = laneStatsMap.begin();
		while(laneIt != laneStatsMap.end())
		{
			if(laneIt->second->laneAgents.size() > 0) {
				return true;
			}
			laneIt++;
		}
		return false;
	}

	double SegmentStats::getPositionOfLastUpdatedAgentInLane(const Lane* lane) {
		return laneStatsMap.find(lane)->second->getPositionOfLastUpdatedAgent();
	}

	void SegmentStats::setPositionOfLastUpdatedAgentInLane(double positionOfLastUpdatedAgentInLane, const Lane* lane) {
		if(lane != laneInfinity) {
			laneStatsMap.find(lane)->second->setPositionOfLastUpdatedAgent(positionOfLastUpdatedAgentInLane);
		}
	}

	unsigned int sim_mob::SegmentStats::getInitialQueueCount(const Lane* lane){
		return laneStatsMap.find(lane)->second->getInitialQueueCount();
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

	void SegmentStats::printAgents() {
		debugMsgs << "\nSegment " << "[" << roadSegment->getStart()->getID() << "," << roadSegment->getEnd()->getID() << "]" << std::endl;
		std::cout << debugMsgs.str();
		debugMsgs.str("");

		for(std::map<const sim_mob::Lane*, sim_mob::LaneStats* >::const_iterator i = laneStatsMap.begin(); i != laneStatsMap.end(); i++) {
			(*i).second->printAgents();
		}
	}

	void LaneStats::printAgents() {
		debugMsgs << "Lane " << lane->getLaneID_str();
		for(std::vector<sim_mob::Agent*>::const_iterator i = laneAgents.begin(); i != laneAgents.end(); i++) {
			debugMsgs << "|" << (*i)->getId();
		}
		debugMsgs <<std::endl;
		debugMsgs << "LaneCopy " << lane->getLaneID_str();
		for(std::vector<sim_mob::Agent*>::const_iterator i = laneAgentsCopy.begin(); i != laneAgentsCopy.end(); i++) {
			debugMsgs << "|" << (*i)->getId();
		}
		debugMsgs <<std::endl;
		std::cout << debugMsgs.str();
		debugMsgs.str("");
	}

	void LaneStats::verifyOrdering() {
		double distance = -1.0;
		for(std::vector<sim_mob::Agent*>::const_iterator i = laneAgents.begin(); i!=laneAgents.end(); i++) {
			if(distance >= (*i)->distanceToEndOfSegment) {
				debugMsgs << "Invariant violated: Ordering of laneAgents does not reflect ordering w.r.t. distance to end of segment."
						<< "\nSegment: [" << lane->getRoadSegment()->getStart()->getID() << "," << lane->getRoadSegment()->getEnd()->getID() << "] "
						<< " length = " << lane->getRoadSegment()->computeLaneZeroLength()
						<< "\nLane: " << lane->getLaneID()
						<< "\nCulprit Agent: " << (*i)->getId();
				debugMsgs << "\nAgents ";
				for(std::vector<sim_mob::Agent*>::const_iterator j = laneAgents.begin(); j != laneAgents.end(); j++) {
					debugMsgs << "|" << (*j)->getId() << "--" << (*j)->distanceToEndOfSegment;
				}
				throw std::runtime_error(debugMsgs.str());
			}
			else {
				distance = (*i)->distanceToEndOfSegment;
			}
		}
	}

}			// end of namespace sim_mob

