/*
 * TaxiDriverFacets.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriverFacets.hpp>
#include "TaxiDriver.hpp"
#include "path/PathSetManager.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "config/MT_Config.hpp"

namespace sim_mob
{

namespace medium
{
	TaxiDriverMovement::TaxiDriverMovement()
	{
		// TODO Auto-generated constructor stub

	}

	TaxiDriverMovement::~TaxiDriverMovement()
	{
		// TODO Auto-generated destructor stub
	}

	void TaxiDriverMovement::frame_init()
	{
		//initialize path of Taxi,set the origin node
		Vehicle* newVeh = new Vehicle(Vehicle::TAXI, sim_mob::TAXI_LENGTH);
		VehicleBase* oldTaxi = parentTaxiDriver->getResource();
		//safe_delete_item(oldTaxi);
		parentTaxiDriver->setResource(newVeh);
		parentTaxiDriver->taxiDriverMode  = CRUISE;
		parentTaxiDriver->driverMode = CRUISE;
		parentDriver->driverMode = CRUISE;
		assignFirstNode();
		selectNextNodeAndLinksWhileCruising();
	}

	void TaxiDriverMovement::setParentTaxiDriver(TaxiDriver * taxiDriver)
	{
		parentTaxiDriver = taxiDriver;
	}

	void TaxiDriverMovement::assignFirstNode()
	{
		const sim_mob::RoadNetwork *roadNetwork = sim_mob::RoadNetwork::getInstance();
		Node *firstNode = roadNetwork->getFirstNode();
		originNode = firstNode;
	}

	const Lane* TaxiDriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats)
	{
		const BusStop* nextTaxiStand; //= routeTracker.getNextStop();
		DriverMode mode = parentTaxiDriver->getDriverMode();
		//if it is Taxi Stand lane
		if(mode==DRIVE_TO_TAXISTAND&&nextTaxiStand && nextSegStats->hasBusStop(nextTaxiStand))
		{
			return nextSegStats->getOutermostLane();
		}
		else
		{
			const Lane* minLane = nullptr;
			double minQueueLength = std::numeric_limits<double>::max();
			double minLength = std::numeric_limits<double>::max();
			double que = 0.0;
			double total = 0.0;

			const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
			const std::vector<Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
			for (std::vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
			{
				const Lane* lane = *lnIt;
				if (!lane->isPedestrianLane())
				{
					if(nextToNextSegStats
							&& !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment()))
					{
						if(parentTaxiDriver->getDriverMode() != CRUISE)
						{
							if(nextLink&&(!nextSegStats->isConnectedToDownstreamLink(nextLink, lane)))
							{
								continue;
							}
						}
						else
						{

							continue;
						}

					}
					else
					{
						if(nextToNextSegStats||parentTaxiDriver->getDriverMode() != CRUISE)
						{
							continue;
						}
					}
					LaneParams *lparams = nextSegStats->getLaneParams(lane);
					total = nextSegStats->getLaneTotalVehicleLength(lane);
					que = nextSegStats->getLaneQueueLength(lane);
					if (minLength > total)
					{
						//if total length of vehicles is less than current minLength
						minLength = total;
						minQueueLength = que;
						minLane = lane;
					}
					else if (minLength == total)
					{
						//if total length of vehicles is equal to current minLength
						if (minQueueLength > que)
						{
							//and if the queue length is less than current minQueueLength
							minQueueLength = que;
							minLane = lane;
						}
					}
				}
			}

			if(!minLane)
			{
				//throw std::runtime_error("best target lane was not set!");
				//TODO: if minLane is null, there is probably no lane connection from any lane in next segment stats to
				// the lanes in the nextToNextSegmentStats. The code in this block is a hack to avoid errors due to this reason.
				//This code must be removed and an error must be thrown here in future.
				for (std::vector<Lane* >::const_iterator lnIt=lanes.begin(); lnIt!=lanes.end(); ++lnIt)
				{
					if (!((*lnIt)->isPedestrianLane()))
					{
						const Lane* lane = *lnIt;
						total = nextSegStats->getLaneTotalVehicleLength(lane);
						que = nextSegStats->getLaneQueueLength(lane);
						if (minLength > total)
						{
							//if total length of vehicles is less than current minLength
							minLength = total;
							minQueueLength = que;
							minLane = lane;
						}
						else if (minLength == total)
						{
							//if total length of vehicles is equal to current minLength
							if (minQueueLength > que)
							{
								//and if the queue length is less than current minQueueLength
								minQueueLength = que;
								minLane = lane;
							}
						}
					}
				}
			}
			return minLane;
		}
	}

	bool TaxiDriverMovement::canGoToNextRdSeg(DriverUpdateParams& params, const SegmentStats* nextSegStats, const Link* nextLink) const
	{
		//return false if the Driver cannot be added during this time tick
		if (params.elapsedSeconds >= params.secondsInTick)
		{
			return false;
		}

		//check if the next road segment has sufficient empty space to accommodate one more vehicle
		if (!nextSegStats)
		{
			return false;
		}

		double enteringVehicleLength = parentTaxiDriver->getResource()->getLengthInM();
		double maxAllowed = nextSegStats->getNumVehicleLanes() * nextSegStats->getLength();
		double total = nextSegStats->getTotalVehicleLength();

		//if the segment is shorter than the vehicle's length and there are no vehicles in the segment just allow the vehicle to pass through
		//this is just an interim arrangement. this segment should either be removed from database or it's length must be updated.
		//if this hack is not in place, all vehicles will start queuing in upstream segments forever.
		//TODO: remove this hack and put permanent fix

		if ((maxAllowed < enteringVehicleLength) && (total <= 0))
		{
			return true;
		}

		bool hasSpaceInNextStats = ((maxAllowed - total) >= enteringVehicleLength);
		return hasSpaceInNextStats;
	}

	void TaxiDriverMovement::reachNextLinkIfPossible(DriverUpdateParams& params)
	{
		const SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
		const SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);
		double departTime = getLastAccept(laneInNextSegment, nextSegStats);
		params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0));
		const Link *nextLink = getNextLinkForLaneChoice(nextSegStats);
		if (canGoToNextRdSeg(params, nextSegStats,nextLink ))
		{
			if (isQueuing)
			{
				removeFromQueue();
			}
			else
			{
				//departFromCurrentTaxiStand();
			}

			currLane = laneInNextSegment;
			pathMover.advanceInPath();
			pathMover.setPositionInSegment(nextSegStats->getLength());

		}
		else
		{
			if (parentTaxiDriver->getResource()->isMoving())
			{
				//Person is in previous segment (should be added to queue if canGoTo failed)
				if (pathMover.getCurrSegStats() == parentTaxiDriver->parent->getCurrSegStats())
				{
					if (currLane)
					{
						if (parentTaxiDriver->parent->isQueuing)
						{
							moveInQueue();
						}
						else
						{
							addToQueue(); // adds to queue if not already in queue
						}
						parentTaxiDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that advance() and setParentData() is called subsequently
					}
				}
				else if (pathMover.getNextSegStats(false) == parentTaxiDriver->parent->getCurrSegStats())
				{
					//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
					//do nothing
				}
				else
				{
					DebugStream << "Driver " << parentTaxiDriver->parent->getId() << "was neither in virtual queue nor in previous segment!" << "\ndriver| segment: "
							<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
							<< pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: " << currLane->getLaneId() << "\nPerson| segment: "
							<< parentTaxiDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|id: "
							<< parentTaxiDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() << "|lane: "
							<< (parentTaxiDriver->parent->getCurrLane() ? parentTaxiDriver->parent->getCurrLane()->getLaneId() : 0) << std::endl;

					throw ::std::runtime_error(DebugStream.str());
				}
				params.elapsedSeconds = params.secondsInTick;
				parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
			}
			else
			{
				//the bus driver is currently serving a stop
				params.elapsedSeconds = params.secondsInTick; //remain in bus stop
				parentTaxiDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
				parentTaxiDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that in the next tick, flowIntoNextLinkIfPossible() is not called in the next tick without requesting for permission again
			}
		}
	}


	bool TaxiDriverMovement::moveToNextSegment(DriverUpdateParams& params)
	{

		const SegmentStats* currSegStat = pathMover.getCurrSegStats();
		bool res = false;
		bool isEndOfLink_CruiseMode = false;
		bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
		if(parentTaxiDriver->getDriverMode() == CRUISE)
		{
			if(pathMover.isEndOfPath())
			{
				isEndOfLink_CruiseMode = true;
			}
		}

		currSegStat = pathMover.getCurrSegStats();
		const SegmentStats* nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);
		const RoadSegment *curRs = (*(pathMover.getCurrSegStats())).getRoadSegment();
		const RoadSegment *nxtRs = (nxtSegStat ? nxtSegStat->getRoadSegment() : nullptr);
		if (curRs && curRs != nxtRs)
		{
			onSegmentCompleted(curRs, nxtRs);
		}

		if (isNewLinkNext ||  isEndOfLink_CruiseMode)
		{
			onLinkCompleted(curRs->getParentLink(), (nxtRs ? nxtRs->getParentLink() : nullptr));
			if(isEndOfLink_CruiseMode)
			{
				selectNextNodeAndLinksWhileCruising();
			}
		}

		//reset these local variables in case path has been changed in onLinkCompleted
		isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
		currSegStat = pathMover.getCurrSegStats();
		nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

		if (!nxtSegStat)
		{
			//vehicle is done
			if( parentTaxiDriver->getDriverMode() == DRIVE_WITH_PASSENGER && pathMover.isEndOfPath())
			{
				//drop off the passenger
				//change the mode to cruise
				//select the neighboring link
				parentTaxiDriver->alightPassenger();
				const DriverMode &mode = CRUISE;
				parentTaxiDriver->taxiDriverMode = mode;
				selectNextNodeAndLinksWhileCruising();
				return true;
			}
			pathMover.advanceInPath();
			if (pathMover.isPathCompleted())
			{

				const Link* currLink = currSegStat->getRoadSegment()->getParentLink();
				double linkExitTime = params.elapsedSeconds + (params.now.ms()/1000.0);
				parentTaxiDriver->parent->currLinkTravelStats.finalize(currLink, linkExitTime, nullptr);
				TravelTimeManager::getInstance()->addTravelTime(parentTaxiDriver->parent->currLinkTravelStats); //in seconds
				currSegStat->getParentConflux()->setLinkTravelTimes(linkExitTime, currLink);
				parentTaxiDriver->parent->currLinkTravelStats.reset();
				currSegStat->getParentConflux()->getLinkStats(currLink).removeEntitiy(parentTaxiDriver->parent);
				setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat);
				currLane = nullptr;
				parentTaxiDriver->parent->setToBeRemoved();
			}
			params.elapsedSeconds = params.secondsInTick;
			return true;
		}

		if (isNewLinkNext||isEndOfLink_CruiseMode)
		{
			parentTaxiDriver->parent->requestedNextSegStats = nxtSegStat;
			parentTaxiDriver->parent->canMoveToNextSegment = Person_MT::NONE;
			if(isEndOfLink_CruiseMode)
			{
				//select the next cruising node and link
				Conflux *parentConflux = currSegStat->getParentConflux();
				parentTaxiDriver->checkPersonsAndPickUpAtNode(params.now,parentConflux);
			}
			return false; // return whenever a new link is to be entered. Seek permission from Conflux.
		}

		const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

		double departTime = getLastAccept(laneInNextSegment, nxtSegStat);
		params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (params.now.ms()/1000.0)); //in seconds

		const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
		if (canGoToNextRdSeg(params, nxtSegStat, nextLink))
		{
			if (isQueuing)
			{
				removeFromQueue();
			}

			setOutputCounter(currLane, (getOutputCounter(currLane, currSegStat) - 1), currSegStat); // decrement from the currLane before updating it
			currLane = laneInNextSegment;
			pathMover.advanceInPath();
			pathMover.setPositionInSegment(nxtSegStat->getLength());
			double segExitTimeSec = params.elapsedSeconds + (params.now.ms()/1000.0);
			setLastAccept(currLane, segExitTimeSec, nxtSegStat);

			const SegmentStats* prevSegStats = pathMover.getPrevSegStats(true); //previous segment is in the same link
			if (prevSegStats
					&& prevSegStats->getRoadSegment() != pathMover.getCurrSegStats()->getRoadSegment())
			{
				// update road segment travel times
				updateScreenlineCounts(prevSegStats, segExitTimeSec);
			}

			res = true;
			parentTaxiDriver->getResource()->setMoving(true);//set moving is set to true here explicitly because the BD could try to move to next segement from within advance and we want the correct moving status there
			sim_mob::BasicLogger& ptTaxiMoveLogger = sim_mob::Logger::log("TaxiSegmentsPath.csv");
			ptTaxiMoveLogger<<parentTaxiDriver->parent->getDatabaseId()<<","<<nxtSegStat->getRoadSegment()->getRoadSegmentId()<<","<<nxtSegStat->getRoadSegment()->getLinkId()<<std::endl;
			advance(params);
		}
		else
		{
			if(parentTaxiDriver->getResource()->isMoving())
			{
				if (isQueuing)
				{
					moveInQueue();
				}
				else
				{
					addToQueue();
				}
			}

			params.elapsedSeconds = params.secondsInTick;
			parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
		}
		return res;
	}

	void TaxiDriverMovement::frame_tick()
	{
		DriverUpdateParams& params = parentTaxiDriver->getParams();
		const DriverMode &mode = parentTaxiDriver->getDriverMode();
		if(parentTaxiDriver->taxiPassenger != nullptr)
		{
			parentTaxiDriver->taxiPassenger->Movement()->frame_tick();
		}
		if(mode == DRIVE_TO_TAXISTAND)
		{

		}

		else if(mode == QUEUING_AT_TAXISTAND)
		{

		}

		else if (mode == DRIVE_WITH_PASSENGER)
		{

		}

		if (mode == CRUISE)
		{
			//if it has reached at the end of the segment

			if (parentTaxiDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
			{
			}

			else if (parentTaxiDriver->parent->canMoveToNextSegment == Person_MT::DENIED)
			{
			}

			if (!parentTaxiDriver->parent->requestedNextSegStats && params.elapsedSeconds < params.secondsInTick)
			{
				DriverMovement::frame_tick();
			}
			else if(parentTaxiDriver->parent->canMoveToNextSegment == Person_MT::NONE)
			{
				// canMoveToNextSegment is set to Granted or Denied only by the conflux
				// If canMoveToNextSegment is not NONE at this point, the bus driver must have remained in VQ
				// Set parent data must not be called if the driver remained in VQ.
				setParentData(params);
			}
		}

	}

	void TaxiDriverMovement::getLinkAndRoadSegments(Node * start ,Node *end,std::vector<RoadSegment*>& segments)
	{
		std::map<unsigned int,Link*> downStreamLinks = start->getDownStreamLinks();
		std::map<unsigned int,Link*>::iterator itr = downStreamLinks.begin();
		while(itr!=downStreamLinks.end())
		{
			Link *link=itr->second;
			Node *toNode = link->getToNode();
			if(toNode == end)
			{
				segments = link->getRoadSegments();
				break;
			}
			itr++;
		}
	}

	Conflux * TaxiDriverMovement::getPrevConflux()
	{
		MT_Config& mtCfg = MT_Config::getInstance();
		std::map<unsigned int,Link*> mapOfUpStreamLinks = originNode->getUpStreamLinks();
		const Conflux* confluxForLnk = mtCfg.getConfluxForNode(originNode);
		int degub =1;
	}

	void TaxiDriverMovement::setPath(std::vector<const SegmentStats*>& path)
	{
		MT_Config& mtCfg = MT_Config::getInstance();
		const Conflux* confluxForLnk = mtCfg.getConfluxForNode(destinationNode);
		if(!isPathInitialized)
		{
			path.erase(path.begin(),path.end());
			//pathMover.erase(pathMover.begin(),pathMover.end());
			for(std::vector<RoadSegment *>::iterator itr = currentRoute.begin(); itr!=currentRoute.end();itr++)
			{
				const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*itr);
				path.insert(path.end(), statsInSegment.begin(), statsInSegment.end());
			}
			pathMover.setPath(path);
			const SegmentStats* firstSegStat = path.front();
			parentTaxiDriver->parent->setCurrSegStats(firstSegStat);
			parentTaxiDriver->parent->setCurrLane(firstSegStat->laneInfinity);
			parentTaxiDriver->parent->distanceToEndOfSegment = firstSegStat->getLength();
			isPathInitialized = true;
		}
		else
		{
			if(path.size()>0)
			{
				path.erase(path.begin(),path.end()-1);
			}

			for(std::vector<RoadSegment *>::iterator itr = currentRoute.begin(); itr!=currentRoute.end();itr++)
			{
				const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*itr);
				path.insert(path.end(), statsInSegment.begin(), statsInSegment.end());
			}

			const SegmentStats* currSegStats = pathMover.getCurrSegStats();
			pathMover.setPath(path);
			pathMover.setSegmentStatIterator(currSegStats);
		}

	}

	Node* TaxiDriverMovement::getCurrentNode()
	{
		return currentNode;
	}

	void TaxiDriverMovement::selectNextNodeAndLinksWhileCruising()
	{
		if(!currentNode)
		{
			currentNode = originNode;
		}
		else
		{
			currentNode = destinationNode;
		}

		const SegmentStats* currSegStats = parentTaxiDriver->parent->getCurrSegStats();
		if(currSegStats)
		{
			const RoadSegment *currentRoadSegment = currSegStats->getRoadSegment();
			if(currentRoadSegment)
			{
				const Link * currLink  = currentRoadSegment->getParentLink();
				if(currLink)
				{
					if(mapOfLinksAndVisitedCounts.find(currLink) == mapOfLinksAndVisitedCounts.end())
					{
						mapOfLinksAndVisitedCounts[currLink] = 1;
					}
					else
					{
						mapOfLinksAndVisitedCounts[currLink] = mapOfLinksAndVisitedCounts[currLink] + 1;
					}
				}
			}
		}
		std::vector<Node*> nodeVector = currentNode->getNeighbouringNodes();
		if(nodeVector.size()>0)
		{
			std::vector<Node*>::iterator itr = nodeVector.begin();
			Node *destNode = (*(itr));
			Node *originNode = currentNode;
			std::map<unsigned int,Link*> mapOfDownStreamLinks = originNode->getDownStreamLinks();
			std::map<unsigned int,Link*>::iterator linkItr = mapOfDownStreamLinks.begin();
			Link *selectedLink = nullptr;
			unsigned int minNumOfVisits = -1;
			while(linkItr != mapOfDownStreamLinks.end())
			{
				Link *link = (*linkItr).second;
				if(mapOfLinksAndVisitedCounts.find(link) == mapOfLinksAndVisitedCounts.end())
				{
					selectedLink = link;
					break;
				}
				else
				{
					if(minNumOfVisits == -1|| mapOfLinksAndVisitedCounts[link] < minNumOfVisits)
					{
						minNumOfVisits = mapOfLinksAndVisitedCounts[link];
						selectedLink = link;
					}
				}
				linkItr++;
			}
			destinationNode = selectedLink->getToNode();
			currentRoute.erase(currentRoute.begin(),currentRoute.end());
			currentRoute.insert(currentRoute.end(),selectedLink->getRoadSegments().begin(),selectedLink->getRoadSegments().end());
			std::vector<const SegmentStats*> path= getMesoPathMover().getPath();
			setPath(path);
		}
		//depth first or breath first
		//select a node
	}

	void TaxiDriverMovement::addRouteChoicePath(std::vector<WayPoint> &currentRouteChoice,Conflux *parentConflux)
	{
		std::vector<const SegmentStats*> path = getMesoPathMover().getPath();
		if( !isPathInitialized )
		{
			path.erase(path.begin(),path.end());
		}
		else
		{
			if(path.size() > 0)
			{
				path.erase(path.begin(),path.end()-1);
			}
		}
		for(std::vector<WayPoint>::const_iterator itr = currentRouteChoice.begin();itr != currentRouteChoice.end();itr++)
		{
			const Link *link = (*itr).link;
			const std::vector<RoadSegment*>& roadSegments = link->getRoadSegments();
			for(std::vector<RoadSegment*>::const_iterator segItr = roadSegments.begin(); segItr != roadSegments.end() ;segItr++)
			{
				const std::vector<SegmentStats*>& segStatsVector = parentConflux->findSegStats(*segItr);
				path.insert(path.end(),segStatsVector.begin(),segStatsVector.end());
			}
		}

		if(!isPathInitialized)
		{
			const SegmentStats* firstSegStat = path.front();
			pathMover.setPath(path);
			parentTaxiDriver->parent->setCurrSegStats(firstSegStat);
			parentTaxiDriver->parent->setCurrLane(firstSegStat->laneInfinity);
			parentTaxiDriver->parent->distanceToEndOfSegment = firstSegStat->getLength();
			isPathInitialized = true;
		}
		else
		{
			const SegmentStats* currSegStats = pathMover.getCurrSegStats();
			pathMover.setPath(path);
			pathMover.setSegmentStatIterator(currSegStats);
		}
	}

	void TaxiDriverMovement::setCruisingMode()
	{
		const DriverMode &mode =  CRUISE;
		parentTaxiDriver->taxiDriverMode = mode;
		parentDriver->driverMode = mode;
		selectNextNodeAndLinksWhileCruising();
	}

	void TaxiDriverMovement::driveToTaxiStand(/*TaxiStand *stand */)
	{
		const DriverMode &mode = DRIVE_TO_TAXISTAND;
		parentTaxiDriver->setDriveMode(mode);
		SubTrip currSubTrip;
		//currSubTrip.origin = assign node or segment
		bool useInSimulationTT = parentTaxiDriver->getParent()->usesInSimulationTravelTime();
		RoadSegment *taxiStandSegment;
		//RoadSegment taxiStandSegment = TaxiStand->getSegment();
		const Link *taxiStandLink = taxiStandSegment->getParentLink();
		Node *destForRouteChoice = taxiStandLink->getFromNode();
		//get the current segment
		const Link *currSegmentParentLink = currSegment->getParentLink();
		Node *originNode = currSegmentParentLink->getToNode();
		//set origin and destination node
		currSubTrip.origin = WayPoint(originNode);
		currSubTrip.destination = WayPoint(destForRouteChoice);
		PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	}


	void TaxiDriverMovement::driveToNode(Node *destinationNode)
	{
		const Link *currSegmentParentLink = currSegment->getParentLink();
		Node *originNode = currSegmentParentLink->getToNode();
		SubTrip currSubTrip;
		currSubTrip.origin = WayPoint(originNode);
		currSubTrip.destination = WayPoint(destinationNode);
		bool useInSimulationTT = parentTaxiDriver->getParent()->usesInSimulationTravelTime();
		std::vector<WayPoint> currentRoute = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	}

	TaxiDriverBehavior::TaxiDriverBehavior()
	{

	}

	TaxiDriverBehavior::~TaxiDriverBehavior()
	{

	}
}
}

