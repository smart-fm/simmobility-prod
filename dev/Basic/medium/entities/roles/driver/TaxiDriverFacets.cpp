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
#include "entities/TaxiStandAgent.hpp"
#include "entities/misc/TaxiTrip.hpp"

namespace sim_mob
{

namespace medium
{
TaxiDriverMovement::TaxiDriverMovement()
{
}

TaxiDriverMovement::~TaxiDriverMovement()
{
}

void TaxiDriverMovement::frame_init()
{
	Vehicle* newVeh = new Vehicle(Vehicle::TAXI, sim_mob::TAXI_LENGTH);
	parentTaxiDriver->setResource(newVeh);
	parentTaxiDriver->taxiDriverMode = CRUISE;
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
	const std::map<unsigned int, Node *>& mapOfIdsVsNode = roadNetwork->getMapOfIdvsNodes();
	Node *firstNode = roadNetwork->getFirstNode();
	const std::vector<TripChainItem *>&tripChain = parentDriver->getParent()->getTripChain();
	const TaxiTrip *taxiTrip = dynamic_cast<const TaxiTrip*>(tripChain[0]);
	originNode = const_cast<Node*>(taxiTrip->origin.node);
}

const Lane* TaxiDriverMovement::getBestTargetLane(const SegmentStats* nextSegStats,const SegmentStats* nextToNextSegStats)
{
	const BusStop* nextTaxiStand; //= routeTracker.getNextStop();
	DriverMode mode = parentTaxiDriver->getDriverMode();
	//if it is Taxi Stand lane
	if (mode == DRIVE_TO_TAXISTAND && nextTaxiStand && nextSegStats->hasBusStop(nextTaxiStand))
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
		if (!minLane)
		{
			for (vector<Lane*>::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
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

void TaxiDriverMovement::setCurrentNode(const Node *node)
{
	currentNode = node;
}

void TaxiDriverMovement::setDestinationNode(const Node *node)
{
	destinationNode = node;
}

void TaxiDriverMovement::setToCallMovementTick(bool toCall)
{
	toCallMovTickInTaxiStand = toCall;
}

bool TaxiDriverMovement::getToCallMovementTick()
{
	return toCallMovTickInTaxiStand;
}


bool TaxiDriverMovement::moveToNextSegment(DriverUpdateParams& params)
{
	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	bool res = false;
	bool passengerDropped = false;
	bool isEndOfLink_CruiseMode = false;
	if (parentTaxiDriver->getDriveMode() == DRIVE_TO_TAXISTAND)
	{
		const SegmentStats* currSegStat = pathMover.getCurrSegStats();
		if (destinationTaxiStand && currSegStat->hasTaxiStand(destinationTaxiStand))
		{
			TaxiStandAgent* taxiStandAgent = TaxiStandAgent::getTaxiStandAgent(destinationTaxiStand);
			if (taxiStandAgent)
			{
				bool canAccomodate = taxiStandAgent->acceptTaxiDriver(parentTaxiDriver->parent);
				if (canAccomodate)
				{
					//add to taxi stand
					if (parentTaxiDriver->getResource()->isMoving())
					{
						if (isQueuing)
						{
							removeFromQueue();
						}
					}
					parentTaxiDriver->setTaxiDriveMode(QUEUING_AT_TAXISTAND);
					parentTaxiDriver->getResource()->setMoving(false);
					parentTaxiDriver->parent->setEnteredTaxiStand(true);
					return res;
				}
				else
				{
					destinationTaxiStand = nullptr;
					//choose some other taxi stand
					//destinationTaxiStand = ?? based on the model
					if (destinationTaxiStand)
					{
						getMesoPathMover().addPathFromCurrentSegmentToEndNodeOfLink();
						driveToTaxiStand();
						return moveToNextSegment(params);
					}
				}
			}
			return res;
		}
	}

	if (parentTaxiDriver->getDriverMode() == CRUISE)
	{
		if (pathMover.isEndOfPath())
		{
			Conflux *parentConflux = currSegStat->getParentConflux();
			parentTaxiDriver->checkPersonsAndPickUpAtNode(parentConflux);
			//if no passengers are found then select the next link from the intersection in cruising mode
			if (parentTaxiDriver->getPassenger() == nullptr)
			{
				selectNextNodeAndLinksWhileCruising();
			}
		}
	}
	else if (parentTaxiDriver->getDriverMode() == DRIVE_WITH_PASSENGER && pathMover.isEndOfPath())
	{
		//drop off the passenger
		//change the mode to cruise
		//select the neighboring link
		parentTaxiDriver->alightPassenger();
		parentTaxiDriver->taxiDriverMode = CRUISE;
		selectNextNodeAndLinksWhileCruising();
	}

	return DriverMovement::moveToNextSegment(params);
}

void TaxiDriverMovement::frame_tick()
{
	DriverUpdateParams& params = parentTaxiDriver->getParams();
	const DriverMode &mode = parentTaxiDriver->getDriverMode();
	if (parentTaxiDriver->taxiPassenger != nullptr)
	{
		parentTaxiDriver->taxiPassenger->Movement()->frame_tick();
	}

	if (mode == QUEUING_AT_TAXISTAND)
	{
		bool isWaitingForTooLong = false;
		bool isPickUp = false;
		TaxiStandAgent *agent = TaxiStandAgent::getTaxiStandAgent(destinationTaxiStand);
		if (agent->isTaxiFirstInQueue(parentTaxiDriver))
		{
			Person_MT * personPickedUp = agent->pickupOneWaitingPerson();
			if (personPickedUp != nullptr)
			{
				//move to destination with passenger
				//run the route choice for destination
				previousTaxiStand = destinationTaxiStand;
				destinationTaxiStand = nullptr;
				isQueuingTaxiStand = false;
				//go to destination with passenger
				//run the route choice model for that
				parentDriver->setDriveMode(DRIVE_WITH_PASSENGER);
				bool movedToNextSegStat = moveToNextSegment(params);
				toBeRemovedFromTaxiStand = true;
			}
			else
			{
				params.elapsedSeconds = params.secondsInTick;
				parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
			}
		}
		else if (isWaitingForTooLong)
		{
			//choose some other Taxi Stand
			//run the route choice model for that
			previousTaxiStand = destinationTaxiStand;
			destinationTaxiStand = nullptr;
			isQueuingTaxiStand = false;
			TaxiStand * newTaxiStand = nullptr;
			destinationTaxiStand = newTaxiStand;
			parentDriver->setDriveMode(DRIVE_TO_TAXISTAND);
			getMesoPathMover().addPathFromCurrentSegmentToEndNodeOfLink();
			driveToTaxiStand();
			bool movedToNextSegment = moveToNextSegment(params);
			if (movedToNextSegment)
			{
				if (parentTaxiDriver->getDriverMode() == QUEUING_AT_TAXISTAND)
				{
					//it could be possible that the taxi got added to other taxi stand
					if (destinationTaxiStand)
					{
						parentTaxiDriver->getResource()->setMoving(false);
					}
				}
			}
			toBeRemovedFromTaxiStand = true;
		}
		else
		{
			waitingTimeAtTaxiStand = waitingTimeAtTaxiStand + params.secondsInTick;
			params.elapsedSeconds = params.secondsInTick;
			parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
		}
	}
	else if (mode == CRUISE || mode == DRIVE_WITH_PASSENGER || mode == DRIVE_TO_TAXISTAND)
	{
		DriverMovement::frame_tick();
	}
}

void TaxiDriverMovement::getLinkAndRoadSegments(Node * start, Node *end,std::vector<RoadSegment*>& segments)
{
	std::map<unsigned int, Link*> downStreamLinks = start->getDownStreamLinks();
	std::map<unsigned int, Link*>::iterator itr = downStreamLinks.begin();
	while (itr != downStreamLinks.end())
	{
		Link *link = itr->second;
		Node *toNode = link->getToNode();
		if (toNode == end)
		{
			segments = link->getRoadSegments();
			break;
		}
		itr++;
	}
}

void TaxiDriverMovement::setPath(std::vector<const SegmentStats*>& path)
{
	MT_Config& mtCfg = MT_Config::getInstance();
	const Conflux* confluxForLnk = mtCfg.getConfluxForNode(destinationNode);
	if (!isPathInitialized)
	{
		path.erase(path.begin(), path.end());
		for (std::vector<RoadSegment *>::iterator itr = currentRoute.begin();itr != currentRoute.end(); itr++)
		{
			const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*itr);
			path.insert(path.end(), statsInSegment.begin(),statsInSegment.end());
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
		if (path.size() > 0)
		{
			path.erase(path.begin(), path.end() - 1);
		}

		for (std::vector<RoadSegment *>::iterator itr = currentRoute.begin();itr != currentRoute.end(); itr++)
		{
			const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*itr);
			path.insert(path.end(), statsInSegment.begin(),statsInSegment.end());
		}

		const SegmentStats* currSegStats = pathMover.getCurrSegStats();
		pathMover.setPath(path);
		pathMover.setSegmentStatIterator(currSegStats);
	}
}

const Node* TaxiDriverMovement::getCurrentNode()
{
	return currentNode;
}

TaxiDriver* TaxiDriverMovement::getParentDriver()
{
	return parentTaxiDriver;
}
void TaxiDriverMovement::selectNextNodeAndLinksWhileCruising()
{
	if (!currentNode)
	{
		currentNode = originNode;
		parentTaxiDriver->currentNode = currentNode;
	}
	else
	{
		currentNode = destinationNode;
		parentTaxiDriver->currentNode = currentNode;
	}

	const SegmentStats* currSegStats = parentTaxiDriver->parent->getCurrSegStats();
	if (currSegStats)
	{
		const RoadSegment *currentRoadSegment = currSegStats->getRoadSegment();
		if (currentRoadSegment)
		{
			const Link * currLink = currentRoadSegment->getParentLink();
			if (currLink)
			{
				if (mapOfLinksAndVisitedCounts.find(currLink) == mapOfLinksAndVisitedCounts.end())
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
	if (nodeVector.size() > 0)
	{
		std::vector<Node*>::iterator itr = nodeVector.begin();
		Node *destNode = (*(itr));
		std::map<unsigned int, Link*> mapOfDownStreamLinks = currentNode->getDownStreamLinks();
		std::map<unsigned int, Link*>::iterator linkItr = mapOfDownStreamLinks.begin();
		Link *selectedLink = nullptr;
		unsigned int minNumOfVisits = -1;
		const Lane *currLane = getCurrentlane();
		const RoadNetwork* rdNetwork = RoadNetwork::getInstance();
		const std::map<const Lane*, std::map<const Lane*, const TurningPath *>>&turningPathsFromLanes = rdNetwork->getTurningPathsFromLanes();
		std::map<const Lane*, std::map<const Lane*, const TurningPath *>>::const_iterator turningPathsLaneitr = turningPathsFromLanes.find(currLane);
		if (!currLane || turningPathsLaneitr != turningPathsFromLanes.end())
		{
			while (linkItr != mapOfDownStreamLinks.end())
			{
				Link *link = (*linkItr).second;
				if (link->getFromNode() == link->getToNode())
				{
					linkItr++;
					continue;
				}
				if (currLane)
				{
					std::map<const Lane*, const TurningPath *> mapOfLaneTurningPath = turningPathsLaneitr->second;
					const std::vector<RoadSegment*>& rdSegments = link->getRoadSegments();
					const RoadSegment* rdSegment = rdSegments.front();
					const std::vector<Lane*> &segLanes = rdSegment->getLanes();
					bool foundTurningPath = false;
					for (std::vector<Lane*>::const_iterator laneItr = segLanes.begin(); laneItr != segLanes.end();laneItr++)
					{
						if (mapOfLaneTurningPath.find(*laneItr)!= mapOfLaneTurningPath.end())
						{
							foundTurningPath = true;
							break;
						}
					}

					if (foundTurningPath == false)
					{
						linkItr++;
						continue;
					}
				}

				if (mapOfLinksAndVisitedCounts.find(link)== mapOfLinksAndVisitedCounts.end())
				{
					selectedLink = link;
					break;
				}
				else if (minNumOfVisits == -1|| mapOfLinksAndVisitedCounts[link] < minNumOfVisits)
				{
					minNumOfVisits = mapOfLinksAndVisitedCounts[link];
					selectedLink = link;
				}
				linkItr++;
			}
			if (selectedLink)
			{
				destinationNode = selectedLink->getToNode();
				parentTaxiDriver->destinationNode = destinationNode;
				currentRoute.erase(currentRoute.begin(), currentRoute.end());
				currentRoute.insert(currentRoute.end(),selectedLink->getRoadSegments().begin(),	selectedLink->getRoadSegments().end());
				std::vector<const SegmentStats*> path =	getMesoPathMover().getPath();
				setPath(path);
			}
		}
	}
}

void TaxiDriverMovement::addRouteChoicePath(std::vector<WayPoint> &currentRouteChoice, Conflux *parentConflux)
{
	std::vector<const SegmentStats*> path = getMesoPathMover().getPath();
	if (!isPathInitialized)
	{
		path.erase(path.begin(), path.end());
	}
	else
	{
		if (path.size() > 0)
		{
			path.erase(path.begin(), path.end() - 1);
		}
	}
	for (std::vector<WayPoint>::const_iterator itr = currentRouteChoice.begin();itr != currentRouteChoice.end(); itr++)
	{
		const Link *link = (*itr).link;
		Node *toNode = link->getToNode();
		Conflux *nodeConflux = Conflux::getConfluxFromNode(toNode);
		const std::vector<RoadSegment*>& roadSegments = link->getRoadSegments();
		for (std::vector<RoadSegment*>::const_iterator segItr =	roadSegments.begin(); segItr != roadSegments.end(); segItr++)
		{
			const std::vector<SegmentStats*>& segStatsVector = nodeConflux->findSegStats(*segItr);
			path.insert(path.end(), segStatsVector.begin(),	segStatsVector.end());
		}
	}

	if (!isPathInitialized)
	{
		const SegmentStats* firstSegStat = path.front();
		pathMover.setPath(path);
		parentTaxiDriver->parent->setCurrSegStats(firstSegStat);
		parentTaxiDriver->parent->setCurrLane(firstSegStat->laneInfinity);
		parentTaxiDriver->parent->distanceToEndOfSegment =firstSegStat->getLength();
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
	parentTaxiDriver->taxiDriverMode = CRUISE;
	parentDriver->driverMode = CRUISE;
	selectNextNodeAndLinksWhileCruising();
}

void TaxiDriverMovement::driveToTaxiStand()
{
	parentTaxiDriver->setDriveMode(DRIVE_TO_TAXISTAND);
	bool useInSimulationTT = parentTaxiDriver->getParent()->usesInSimulationTravelTime();
	const RoadSegment *taxiStandSegment = destinationTaxiStand->getRoadSegment();
	const Link *taxiStandLink = taxiStandSegment->getParentLink();
	Node *destForRouteChoice = taxiStandLink->getFromNode();
	//get the current segment
	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const Link *currSegmentParentLink = currSegStat->getRoadSegment()->getParentLink();
	Node *originNode = currSegmentParentLink->getToNode();
	//set origin and destination node
	SubTrip currSubTrip;
	currSubTrip.origin = WayPoint(originNode);
	currSubTrip.destination = WayPoint(destForRouteChoice);
	vector<WayPoint> routeToTaxiStand = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, nullptr, useInSimulationTT);
	addTaxiStandPath(routeToTaxiStand);
}

void TaxiDriverMovement::addTaxiStandPath(vector<WayPoint> &routeToTaxiStand)
{
	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const Link *currSegmentParentLink = currSegStat->getRoadSegment()->getParentLink();
	const RoadSegment * currRoadSegment = currSegStat->getRoadSegment();
	const std::vector<const SegmentStats*> currPathSegs = pathMover.getPath();
	pathMover.erasePathAfterCurrenrLink();
	pathMover.appendRoute(routeToTaxiStand);
	const RoadSegment *roadSegment = destinationTaxiStand->getRoadSegment();
	const Link * parentLink = roadSegment->getParentLink();
	Node * toNode = parentLink->getToNode();
	Conflux *conflux = Conflux::getConfluxFromNode(toNode);
	const std::vector<RoadSegment*>& roadSegments = parentLink->getRoadSegments();
	std::vector<RoadSegment*> roadSegmentTillTaxiStand;
	std::vector<RoadSegment*>::const_iterator rdSegItr = std::find(roadSegments.begin(), roadSegments.end(), roadSegment);
	roadSegmentTillTaxiStand.insert(roadSegmentTillTaxiStand.end(),roadSegments.begin(), rdSegItr + 1);
	pathMover.appendSegmentStats(roadSegmentTillTaxiStand, conflux);
	pathMover.setSegmentStatIterator(currSegStat);
}

bool TaxiDriverMovement::isToBeRemovedFromTaxiStand()
{
	return toBeRemovedFromTaxiStand;
}

TaxiDriverBehavior::TaxiDriverBehavior()
{

}

TaxiDriverBehavior::~TaxiDriverBehavior()
{

}
}
}

