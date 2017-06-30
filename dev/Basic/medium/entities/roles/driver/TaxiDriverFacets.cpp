/*
 * TaxiDriverFacets.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: zhang huai peng
 */

#include <entities/roles/driver/TaxiDriverFacets.hpp>
#include "config/MT_Config.hpp"
#include "entities/misc/TaxiTrip.hpp"
#include "entities/TaxiStandAgent.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"
#include "TaxiDriver.hpp"

using namespace sim_mob;
using namespace medium;
using namespace messaging;

/**define the threshold for long time waiting, timeout is 15 minutes in seconds*/
const double timeoutForLongWaiting = 15 * 60;

/* taxy Trajectory file*/
sim_mob::BasicLogger& taxitrajectoryLogger  = sim_mob::Logger::log("Taxi_trajectory.csv");


TaxiDriverMovement::TaxiDriverMovement()
{
}

TaxiDriverMovement::~TaxiDriverMovement()
{
}

void TaxiDriverMovement::frame_init()
{
	Vehicle *newVeh = new Vehicle(Vehicle::TAXI, sim_mob::TAXI_LENGTH);
	parentTaxiDriver->setResource(newVeh);
	parentTaxiDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);

	assignFirstNode();

	FleetController::FleetTimePriorityQueue &fleets = parentTaxiDriver->parent->getTaxiFleet();
	currentFleetItem = fleets.top();
	fleets.pop();

	if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
		subscribeToController(controllers, SERVICE_CONTROLLER_GREEDY);
		subscribeToController(controllers, SERVICE_CONTROLLER_SHARED);
		subscribeToController(controllers, SERVICE_CONTROLLER_ON_HAIL);
	}

	(isSubscribedToOnHail() && CruiseOnlyOrMoveToTaxiStand())?driveToTaxiStand():selectNextLinkWhileCruising();  // for 1 : drive_to_taxiStand or cruise


	while (fleets.size() > 0)
	{
		FleetController::FleetItem fleet = fleets.top();
		fleets.pop();
		taxiFleets.push(fleet);
	}
}

void TaxiDriverMovement::subscribeToController(
		multimap<MobilityServiceControllerType, MobilityServiceController *> &controllers,
		MobilityServiceControllerType controllerType)
{
	if (currentFleetItem.controllerSubscription & controllerType)
	{
		auto range = controllers.equal_range(controllerType);
		for (auto itController = range.first; itController != range.second; ++itController)
		{

#ifndef NDEBUG
			if (!isMobilityServiceDriver(parentDriver->getParent()))
			{
				std::stringstream msg;
				msg << "Driver " << parentDriver->getParent()->getDatabaseId()
				    << " is trying to send a subscription message but she is not a MobilityServiceDriver"
				    << std::endl;
				throw std::runtime_error(msg.str());
			}
#endif

			MessageBus::PostMessage(itController->second, MSG_DRIVER_SUBSCRIBE,
			                        MessageBus::MessagePtr(new DriverSubscribeMessage(parentTaxiDriver->getParent())));
#ifndef NDEBUG
			ControllerLog() << "Driver " << parentDriver->getParent()->getDatabaseId()
			                << " sent a subscription to the controller "
			                << itController->second->GetId() << " at time " << parentDriver->getParent()->currTick
			                << std::endl;
#endif

			subscribedControllers.push_back(itController->second);
		}
	}
}

void TaxiDriverMovement::setParentTaxiDriver(TaxiDriver *taxiDriver)
{
	parentTaxiDriver = taxiDriver;
}

void TaxiDriverMovement::assignFirstNode()
{
	const sim_mob::RoadNetwork *roadNetwork = sim_mob::RoadNetwork::getInstance();
	const std::map<unsigned int, Node *> &mapOfIdsVsNode = roadNetwork->getMapOfIdvsNodes();
	Node *firstNode = roadNetwork->getFirstNode();
	const std::vector<TripChainItem *> &tripChain = parentDriver->getParent()->getTripChain();
	const TaxiTrip *taxiTrip = dynamic_cast<const TaxiTrip *>(tripChain[0]);
	originNode = const_cast<Node *>(taxiTrip->origin.node);
}

const Lane *
TaxiDriverMovement::getBestTargetLane(const SegmentStats *nextSegStats, const SegmentStats *nextToNextSegStats)
{
	MobilityServiceDriverStatus mobilityServiceDriverStatus = parentTaxiDriver->getDriverStatus();
	if (mobilityServiceDriverStatus == DRIVE_TO_TAXISTAND && destinationTaxiStand && nextSegStats->hasTaxiStand(
			destinationTaxiStand))
	{
		return nextSegStats->getOutermostLane();
	}
	else
	{
		const Lane *minLane = nullptr;
		double minQueueLength = std::numeric_limits<double>::max();
		double minLength = std::numeric_limits<double>::max();
		double que = 0.0;
		double total = 0.0;
		const Link *nextLink = getNextLinkForLaneChoice(nextSegStats);
		const std::vector<Lane *> &lanes = nextSegStats->getRoadSegment()->getLanes();
		if (!minLane)
		{
			for (vector<Lane *>::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
			{
				if (!((*lnIt)->isPedestrianLane()))
				{
					const Lane *lane = *lnIt;
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

const Node *TaxiDriverMovement::getCurrentNode()
{
	return currentNode;
}

void TaxiDriverMovement::setDestinationNode(const Node *node)
{
	destinationNode = node;
}

const Node *TaxiDriverMovement::getDestinationNode()
{
	return destinationNode;
}

bool TaxiDriverMovement::moveToNextSegment(DriverUpdateParams &params)
{
	const SegmentStats *currSegStat = pathMover.getCurrSegStats();
	const SegmentStats *nxtSegStat = pathMover.getNextSegStats(false);
	bool res = false;

	if (parentTaxiDriver->getDriverStatus() == DRIVE_TO_TAXISTAND)
	{
		const SegmentStats *currSegStat = pathMover.getCurrSegStats();
		if (destinationTaxiStand && currSegStat->hasTaxiStand(destinationTaxiStand))
		{
			TaxiStandAgent *taxiStandAgent = TaxiStandAgent::getTaxiStandAgent(destinationTaxiStand);
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
					parentTaxiDriver->setDriverStatus(QUEUING_AT_TAXISTAND);
					parentTaxiDriver->getResource()->setMoving(false);
					params.elapsedSeconds = params.secondsInTick;
					parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
					toBeRemovedFromTaxiStand = false;
					return res;
				}
				else
				{
					setCruisingMode();
					return moveToNextSegment(params);
				}
			}
			return res;
		}
	}

	if (parentTaxiDriver->getDriverStatus() == CRUISING && isSubscribedToOnHail())
	{
		if (cruisingTooLongTime > timeoutForLongWaiting)
		{
			cruisingTooLongTime = 0.0;
            driveToTaxiStand();
		}
		if (pathMover.isEndOfPath())
		{
			Conflux *parentConflux = currSegStat->getParentConflux();
			parentTaxiDriver->pickUpPassngerAtNode(parentConflux);
			if (parentTaxiDriver->getPassenger() == nullptr)
			{
				selectNextLinkWhileCruising();
			}
		}
	}
	else if (parentTaxiDriver->getDriverStatus() == CRUISING && !isSubscribedToOnHail())
	{
		if (pathMover.isEndOfPath())
		{
			selectNextLinkWhileCruising();
		}
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_WITH_PASSENGER && pathMover.isEndOfPath())
	{
		parentTaxiDriver->alightPassenger();

		if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
		{
			for (auto it = subscribedControllers.begin(); it != subscribedControllers.end(); ++it)
			{
				MessageBus::PostMessage(*it, MSG_DRIVER_AVAILABLE,
				                        MessageBus::MessagePtr(new DriverAvailableMessage(parentTaxiDriver->parent)));
			}
		}
        if(isSubscribedToOnHail())
        {
            if (CruiseOnlyOrMoveToTaxiStand())      //Decision point.Logic Would be Replaced as per Bathen's Input
            {
                parentTaxiDriver->setDriverStatus(CRUISING);
                selectNextLinkWhileCruising();
            }
            else
            {
                driveToTaxiStand();
            }
        }
        else
        {
            parentTaxiDriver->setDriverStatus(CRUISING);
            selectNextLinkWhileCruising();
        }
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_FOR_DRIVER_CHANGE_SHIFT && pathMover.isEndOfPath())
	{
		setCruisingMode();
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_FOR_BREAK && pathMover.isEndOfPath())
	{
		if (setBreakMode())
		{
			return res;
		}
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_ON_CALL && pathMover.isEndOfPath())
	{
		Conflux *parentConflux = currSegStat->getParentConflux();
		parentTaxiDriver->pickUpPassngerAtNode(parentConflux, &personIdPickedUp);

		if (parentTaxiDriver->getPassenger() == nullptr)
		{
			ControllerLog() << "Pickup failed for " << personIdPickedUp << " at time "
			                << parentTaxiDriver->parent->currTick
			                << ". Message was sent at ??? with startNodeId "
			                << parentConflux->getConfluxNode()->getNodeId() << ", destinationNodeId ???"
			                << ", and driverId " << parentTaxiDriver->parent->getDatabaseId() << std::endl;

			setCruisingMode();
		}
		else
		{
			ControllerLog() << "Pickup succeeded for " << personIdPickedUp << " at time "
			                << parentTaxiDriver->parent->currTick
			                << ". Message was sent at ??? with startNodeId "
			                << parentConflux->getConfluxNode()->getNodeId() << ", destinationNodeId "
			                << destinationNode->getNodeId()
			                << ", and driverId " << parentTaxiDriver->parent->getDatabaseId() << std::endl;
		}
	}

	res = DriverMovement::moveToNextSegment(params);

	return res;
}

bool TaxiDriverMovement::isSubscribedToOnHail() const
{
	if (currentFleetItem.controllerSubscription & MobilityServiceControllerType::SERVICE_CONTROLLER_ON_HAIL)
	{
		return true;
	}
	else if (currentFleetItem.controllerSubscription & MobilityServiceControllerType::SERVICE_CONTROLLER_UNKNOWN)
	{
		stringstream msg;
		msg << "Driver " << currentFleetItem.driverId << " subscribes to unknown service controller";
		throw runtime_error(msg.str());
	}
	else
	{
		return false;
	}
}

bool TaxiDriverMovement::checkNextFleet()
{
	bool res = false;
	DriverUpdateParams &params = parentTaxiDriver->getParams();

	if (taxiFleets.size() > 0 && isSubscribedToOnHail())
	{
		currentFleetItem = taxiFleets.front();
		if (currentFleetItem.startTime < params.now.ms() / 1000.0)
		{
			const Link *link = this->currLane->getParentSegment()->getParentLink();
			SubTrip currSubTrip;
			currSubTrip.origin = WayPoint(link->getFromNode());
			currSubTrip.destination = WayPoint(currentFleetItem.startNode);
			std::vector<WayPoint> currentRouteChoice =
					PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, link,
					                                                  parentTaxiDriver->parent->usesInSimulationTravelTime());

			if (currentRouteChoice.size() > 0)
			{
				res = true;
				taxiFleets.pop();
				currentNode = link->getFromNode();
				destinationNode = currentFleetItem.startNode;
				setCurrentNode(currentNode);
				setDestinationNode(destinationNode);
				addRouteChoicePath(currentRouteChoice);
				parentTaxiDriver->parent->setDatabaseId(currentFleetItem.driverId);

				parentTaxiDriver->setDriverStatus(DRIVE_FOR_DRIVER_CHANGE_SHIFT);

				if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
				{
					for (auto it = subscribedControllers.begin(); it != subscribedControllers.end(); ++it)
					{
						MessageBus::PostMessage(*it, MSG_DRIVER_UNSUBSCRIBE, MessageBus::MessagePtr(
								new DriverUnsubscribeMessage(parentTaxiDriver->getParent())));
					}
				}

			}
		}
	}
	return res;
}

void TaxiDriverMovement::frame_tick()
{
	DriverUpdateParams &params = parentTaxiDriver->getParams();
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();
	if (parentTaxiDriver->taxiPassenger != nullptr)
	{
		parentTaxiDriver->taxiPassenger->Movement()->frame_tick();
	}

	if (mode == DRIVER_IN_BREAK && nextBroken.get())
	{
		if (nextBroken->duration < params.secondsInTick)
        {
            parentTaxiDriver->getResource()->setMoving(true);
            Conflux *nodeConflux = Conflux::getConfluxFromNode(nextBroken->parkingNode);
            nodeConflux->removeBrokenDriver(parentTaxiDriver->parent);
            nextBroken.reset();
			(isSubscribedToOnHail() && CruiseOnlyOrMoveToTaxiStand())?driveToTaxiStand():setCruisingMode();  // for 1 : drive_to_taxiStand or cruise
		}
		else
		{
			nextBroken->duration -= params.secondsInTick;
		}
		return;
	}
	if (mode == CRUISING && isSubscribedToOnHail())
	{
		cruisingTooLongTime += params.secondsInTick;
	}
	if (mode == QUEUING_AT_TAXISTAND)
	{
		queuingTooLongTime += params.secondsInTick;
		TaxiStandAgent *agent = TaxiStandAgent::getTaxiStandAgent(destinationTaxiStand);
		if (agent->isTaxiFirstInQueue(parentTaxiDriver))
		{
			Person_MT *personPickedUp = agent->pickupOneWaitingPerson();
			if (personPickedUp != nullptr)
			{
				std::string id = personPickedUp->getDatabaseId();
				Role<Person_MT> *curRole = personPickedUp->getRole();
				sim_mob::medium::Passenger *passenger = dynamic_cast<sim_mob::medium::Passenger *>(curRole);
				if (passenger)
				{
					std::vector<SubTrip>::iterator subTripItr = personPickedUp->currSubTrip;
					WayPoint personTravelDestination = (*subTripItr).destination;
					const Node *personDestinationNode = personTravelDestination.node;
					std::vector<WayPoint> currentRouteChoice;
					currentNode = destinationNode;
					if (currentNode != personDestinationNode)
					{
						const Link *link = destinationTaxiStand->getRoadSegment()->getParentLink();
						SubTrip currSubTrip;
						currSubTrip.origin = WayPoint(link->getFromNode());
						currSubTrip.destination = WayPoint(personDestinationNode);
						currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, link,
						                                                                       parentTaxiDriver->parent->usesInSimulationTravelTime());
						if (currentRouteChoice.size() == 0)
						{
							currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false,
							                                                                       nullptr,
							                                                                       parentTaxiDriver->parent->usesInSimulationTravelTime());
							if (currentRouteChoice.size() > 0)
							{
								const Link *destLink = nullptr;
								if (currentRouteChoice.front().type == WayPoint::LINK)
								{
									destLink = currentRouteChoice.front().link;
								}
								if (destLink)
								{
									vector<WayPoint> path = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Link>(
											*link, *destLink);
									vector<WayPoint> pathOfLinks;
									for (vector<WayPoint>::iterator itWayPts = path.begin(); itWayPts != path.end();
									     ++itWayPts)
									{
										if (itWayPts->type == WayPoint::LINK)
										{
											pathOfLinks.push_back(*itWayPts);
										}
									}
									if (pathOfLinks.size() > 0)
									{
										const Link *lastLink = pathOfLinks.back().link;
										pathOfLinks.erase(pathOfLinks.end() - 1);
										currentRouteChoice.insert(currentRouteChoice.begin(), pathOfLinks.begin(),
										                          pathOfLinks.end());
									}
									else
									{
										currentRouteChoice.clear();
									}
								}
							}
							if (currentRouteChoice.size() > 0)
							{
								bool isAdded = parentTaxiDriver->addPassenger(passenger);
								if (isAdded)
								{
									destinationNode = personDestinationNode;
									setCurrentNode(currentNode);
									setDestinationNode(destinationNode);
									addRouteChoicePath(currentRouteChoice);
									passenger->setStartPoint(WayPoint(destinationTaxiStand));
									passenger->setEndPoint(WayPoint(destinationNode));
									parentTaxiDriver->setDriverStatus(DRIVE_WITH_PASSENGER);
									parentTaxiDriver->getResource()->setMoving(true);
									toBeRemovedFromTaxiStand = true;
									previousTaxiStand = destinationTaxiStand;
									destinationTaxiStand = nullptr;
									passenger->Movement()->startTravelTimeMetric();
								}
							}
						}
					}
				}
			}
			else if (queuingTooLongTime > timeoutForLongWaiting)
			{
				setCruisingMode();
				parentTaxiDriver->getResource()->setMoving(true);
				toBeRemovedFromTaxiStand = true;
			}
		}
		params.elapsedSeconds = params.secondsInTick;
		parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
	}

	if (mode != QUEUING_AT_TAXISTAND && mode != DRIVER_IN_BREAK)
	{
		DriverMovement::frame_tick();
	}
}

bool TaxiDriverMovement::setBreakMode()
{
	bool res = false;
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();
	if (mode != DRIVE_FOR_BREAK)
	{
		return res;
	}
	if (nextBroken.get())
	{
		const SegmentStats *currSegStat = pathMover.getCurrSegStats();
		const Node *toNode = currSegStat->getRoadSegment()->getParentLink()->getToNode();
		if (toNode == nextBroken->parkingNode)
		{
			parentTaxiDriver->setDriverStatus(DRIVER_IN_BREAK);
			parentTaxiDriver->getResource()->setMoving(false);
			parentTaxiDriver->parent->setRemainingTimeThisTick(0.0);
			Conflux *nodeConflux = Conflux::getConfluxFromNode(toNode);
			nodeConflux->acceptBrokenDriver(parentTaxiDriver->parent);
			res = true;
		}
	}
	return res;
}

bool TaxiDriverMovement::setBreakInfo(const Node *next, const unsigned int duration)
{
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();
	if (mode != CRUISING)
	{
		return false;
	}
	const SegmentStats *currSegStat = pathMover.getCurrSegStats();
	const Link *link = currSegStat->getRoadSegment()->getParentLink();
	SubTrip currSubTrip;
	currSubTrip.origin = WayPoint(link->getFromNode());
	currSubTrip.destination = WayPoint(next);
	vector<WayPoint> currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, link,
	                                                                                        parentTaxiDriver->parent->usesInSimulationTravelTime());
	if (currentRouteChoice.size() > 0)
	{
		addRouteChoicePath(currentRouteChoice);
		nextBroken.reset();
		nextBroken = std::make_shared<BrokenInfo>();
		nextBroken->parkingNode = next;
		nextBroken->duration = duration;
		parentTaxiDriver->setDriverStatus(DRIVE_FOR_BREAK);
		return true;
	}
	return false;
}

void TaxiDriverMovement::addCruisingPath(const Link *selectedLink)
{
	MT_Config &mtCfg = MT_Config::getInstance();
	std::vector<const SegmentStats *> path = getMesoPathMover().getPath();
	std::vector<RoadSegment *> currentRoute;
	currentRoute.insert(currentRoute.end(), selectedLink->getRoadSegments().begin(),
	                    selectedLink->getRoadSegments().end());
	const Conflux *confluxForLnk = mtCfg.getConfluxForNode(destinationNode);
	if (!isPathInitialized)
	{
		path.erase(path.begin(), path.end());
		for (std::vector<RoadSegment *>::iterator itr = currentRoute.begin(); itr != currentRoute.end(); itr++)
		{
			const vector<SegmentStats *> &statsInSegment = confluxForLnk->findSegStats(*itr);
			path.insert(path.end(), statsInSegment.begin(), statsInSegment.end());
		}
		pathMover.setPath(path);
		const SegmentStats *firstSegStat = path.front();
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

		for (std::vector<RoadSegment *>::iterator itr = currentRoute.begin(); itr != currentRoute.end(); itr++)
		{
			const vector<SegmentStats *> &statsInSegment = confluxForLnk->findSegStats(*itr);
			path.insert(path.end(), statsInSegment.begin(), statsInSegment.end());
		}

		const SegmentStats *currSegStats = pathMover.getCurrSegStats();
		pathMover.setPath(path);
		pathMover.setSegmentStatIterator(currSegStats);
	}
}

TaxiDriver *TaxiDriverMovement::getParentDriver()
{
	return parentTaxiDriver;
}

void TaxiDriverMovement::selectNextLinkWhileCruising()
{
	if (checkNextFleet())
	{
		return;
	}

	if (!currentNode)
	{
		currentNode = originNode;
	}
	else
	{
		currentNode = destinationNode;
	}

	const SegmentStats *currSegStats = parentTaxiDriver->parent->getCurrSegStats();
	if (currSegStats)
	{
		const RoadSegment *currentRoadSegment = currSegStats->getRoadSegment();
		if (currentRoadSegment)
		{
			const Link *currLink = currentRoadSegment->getParentLink();
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

	std::stringstream msg;
	selectedNextLinkInCrusing = nullptr;
	std::vector<Node *> nodeVector = currentNode->getNeighbouringNodes();
	if (nodeVector.size() > 0)
	{
		std::vector<Node *>::iterator itr = nodeVector.begin();
		Node *destNode = (*(itr));
		std::map<unsigned int, Link *> mapOfDownStreamLinks = currentNode->getDownStreamLinks();
		std::map<unsigned int, Link *>::iterator linkItr = mapOfDownStreamLinks.begin();
		const SegmentStats *currSegStats = pathMover.getCurrSegStats();
		int minNumOfVisits = -1;
		const Lane *currLane = getCurrentlane();
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
		const std::map<const Lane *, std::map<const Lane *, const TurningPath *>> &turningPathsFromLanes = rdNetwork->getTurningPathsFromLanes();
		std::map<const Lane *, std::map<const Lane *, const TurningPath *>>::const_iterator turningPathsLaneitr = turningPathsFromLanes.find(
				currLane);
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
					std::map<const Lane *, const TurningPath *> mapOfLaneTurningPath = turningPathsLaneitr->second;
					const std::vector<RoadSegment *> &rdSegments = link->getRoadSegments();
					const RoadSegment *rdSegment = rdSegments.front();
					const std::vector<Lane *> &segLanes = rdSegment->getLanes();
					bool foundTurningPath = false;
					for (std::vector<Lane *>::const_iterator laneItr = segLanes.begin(); laneItr != segLanes.end();
					     laneItr++)
					{
						if (mapOfLaneTurningPath.find(*laneItr) != mapOfLaneTurningPath.end())
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

				if (mapOfLinksAndVisitedCounts.find(link) == mapOfLinksAndVisitedCounts.end())
				{
					selectedNextLinkInCrusing = link;
					break;
				}
				else if (minNumOfVisits == -1 || mapOfLinksAndVisitedCounts[link] < minNumOfVisits)
				{
					minNumOfVisits = mapOfLinksAndVisitedCounts[link];
					selectedNextLinkInCrusing = link;
				}
				linkItr++;
			}
		}
	}

	if (selectedNextLinkInCrusing)
	{
		destinationNode = selectedNextLinkInCrusing->getToNode();
		addCruisingPath(selectedNextLinkInCrusing);
	}

}

void TaxiDriverMovement::setCruisingMode()
{
	previousTaxiStand = destinationTaxiStand;
	destinationTaxiStand = nullptr;
	parentTaxiDriver->setDriverStatus(CRUISING);

	if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		for (auto it = subscribedControllers.begin(); it != subscribedControllers.end(); ++it)
		{
			MessageBus::PostMessage(*it, MSG_DRIVER_AVAILABLE,
			                        MessageBus::MessagePtr(new DriverAvailableMessage(parentTaxiDriver->parent)));
		}
	}

	if (pathMover.isEndOfPath())
	{
		selectNextLinkWhileCruising();
	}
}

bool TaxiDriverMovement::driveToNodeOnCall(const std::string &personId, const Node *destination)
{
	bool res = false;
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();
	if (mode == CRUISING && destination)
	{
		const Link *link = this->currLane->getParentSegment()->getParentLink();
		std::vector<WayPoint> currentRouteChoice =
				StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*link, *destination);

		if (currentRouteChoice.size() > 0)
		{
			res = true;
			currentNode = link->getFromNode();
			destinationNode = destination;
			setCurrentNode(currentNode);
			setDestinationNode(destinationNode);
			addRouteChoicePath(currentRouteChoice);
			parentTaxiDriver->setDriverStatus(DRIVE_ON_CALL);
			personIdPickedUp = personId;
		}
	}

	if (!res)
	{
		if (mode != CRUISING)
		{
			ControllerLog() << "Assignment failed for " << personId << " because mode was not CRUISING" << std::endl;
		}
		else if (!destination)
		{
			ControllerLog() << "Assignment failed for " << personId << " because destination was null" << std::endl;
		}
		else
		{
			ControllerLog() << "Assignment failed for " << personId << " because currentRouteChoice was empty"
			                << ". No path from lane " << this->currLane->getLaneId() << " to node "
			                << destination->getNodeId() << std::endl;
		}
	}

	return res;
}

void TaxiDriverMovement::cruiseToNode(const Node *destination)
{
	bool result = false;
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();

	if (mode != DRIVE_WITH_PASSENGER && destination)
	{
		const Link *currLink = this->currLane->getParentSegment()->getParentLink();
		SubTrip subTrip;
		subTrip.origin = WayPoint(currLink->getFromNode());
		subTrip.destination = WayPoint(destination);
		std::vector<WayPoint> pathToNode = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink,
		                                                                                     parentTaxiDriver->getParent()->usesInSimulationTravelTime());

		if (!pathToNode.empty())
		{
			result = true;
			currentNode = currLink->getFromNode();
			destinationNode = destination;
			setCurrentNode(currentNode);
			setDestinationNode(destinationNode);
			addRouteChoicePath(pathToNode);
			parentTaxiDriver->setDriverStatus(CRUISING);
		}
		else
		{
			stringstream msg;
			msg << "Unable to CRUISE to node " << destination->getNodeId() << ". No path available from "
			    << "current link " << currLink->getLinkId();
			throw runtime_error(msg.str());
		}
	}
	else if(destination == nullptr)
	{
		stringstream msg;
		msg << "Invalid destination node. Unable to CRUISE to node.";
		throw runtime_error(msg.str());
	}
	else
	{
		stringstream msg;
		msg << "Unable to CRUISE to node " << destination->getNodeId()
		    << ". Current mode is DRIVE_WITH_PASSENGER";
		throw runtime_error(msg.str());
	}
}

void TaxiDriverMovement::driveToTaxiStand()
{
    bool useInSimulationTT = parentTaxiDriver->getParent()->usesInSimulationTravelTime();
    const Link *currSegmentParentLink = NULL;
    const Node *ThisNode;

	if(!currentNode)
    {
        currentNode=originNode;
        const Point &pointFromNode = currentNode->getLocation();
        destinationTaxiStand = TaxiStand::allTaxiStandMap.searchNearestObject(pointFromNode.getX(),
                                                                              pointFromNode.getY());
		ThisNode = currentNode;
    }
    else
    {
        const SegmentStats *currSegStat = pathMover.getCurrSegStats();
        currSegmentParentLink = currSegStat->getRoadSegment()->getParentLink();
        const PolyPoint point = currSegStat->getRoadSegment()->getPolyLine()->getLastPoint();
        destinationTaxiStand = TaxiStand::allTaxiStandMap.searchNearestObject(point.getX(), point.getY());
        ThisNode = currSegmentParentLink->getFromNode();
    }

    if (!destinationTaxiStand)
    {
        parentTaxiDriver->setDriverStatus(CRUISING);
        selectNextLinkWhileCruising();
    }
    else
    {
        const RoadSegment *taxiStandSegment = destinationTaxiStand->getRoadSegment();
        const Link *taxiStandLink = taxiStandSegment->getParentLink();
        Node *destForRouteChoice = taxiStandLink->getToNode();
        //set origin and destination node
        SubTrip currSubTrip;
        currSubTrip.origin = WayPoint(ThisNode);
        currSubTrip.destination = WayPoint(destForRouteChoice);
        vector<WayPoint> routeToTaxiStand = PrivateTrafficRouteChoice::getInstance()->getPathWhereToStand(currSubTrip,
                                                                                                          false,
                                                                                                          currSegmentParentLink,
                                                                                                          nullptr,
                                                                                                          taxiStandLink,
                                                                                                          useInSimulationTT);
        if (routeToTaxiStand.empty() || currSegmentParentLink == taxiStandLink)
        {
            setCruisingMode();
        }
        else
        {
            queuingTooLongTime = 0.0;
            parentTaxiDriver->setDriverStatus(DRIVE_TO_TAXISTAND);
            destinationNode = destForRouteChoice;
            addRouteChoicePath(routeToTaxiStand);
        }
    }
}

void TaxiDriverMovement::addRouteChoicePath(vector<WayPoint> &routeToDestination)
{
	//const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	std::vector<const SegmentStats*> path;
	for (std::vector<WayPoint>::const_iterator itr = routeToTaxiStand.begin();itr != routeToTaxiStand.end(); itr++){

		if((*itr).type==WayPoint::LINK)
			{const Link *link = (*itr).link;
			Node *toNode = link->getToNode();
			Conflux *nodeConflux = Conflux::getConfluxFromNode(toNode);
			const std::vector<RoadSegment*>& roadSegments = link->getRoadSegments();
			for (std::vector<RoadSegment*>::const_iterator segItr =	roadSegments.begin(); segItr != roadSegments.end(); segItr++){

				const std::vector<SegmentStats*>& segStatsVector = nodeConflux->findSegStats(*segItr);
				path.insert(path.end(), segStatsVector.begin(),	segStatsVector.end());
			}
		}
	}if (currentNode == originNode && pathMover.getPath().empty()) {
        pathMover.setPath(path);
        const SegmentStats *firstSegStat = path.front();
        parentTaxiDriver->parent->setCurrSegStats(firstSegStat);
        parentTaxiDriver->parent->setCurrLane(firstSegStat->laneInfinity);
        isPathInitialized = true;
        pathMover.setSegmentStatIterator(firstSegStat);
    }
    else
    {   const SegmentStats *currSegStat = pathMover.getCurrSegStats();
	pathMover.setPath(path);
	pathMover.setSegmentStatIterator(currSegStat);
}
}

bool TaxiDriverMovement::isToBeRemovedFromTaxiStand()
{
	return toBeRemovedFromTaxiStand;
}


const std::vector<MobilityServiceController *> &TaxiDriverMovement::getSubscribedControllers() const
{
	return subscribedControllers;
}

TaxiDriverBehavior::TaxiDriverBehavior()
{

}

TaxiDriverBehavior::~TaxiDriverBehavior()
{

}

std::string TaxiDriverMovement::frame_tick_output()
{

    const DriverUpdateParams& params = parentDriver->getParams();
    if (pathMover.isPathCompleted() || ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
    {
        return std::string();
    }

    std::ostringstream out(" ");
    if(originNode==currentNode && params.now.ms()==(uint32_t)0) {

        out << currentFleetItem.vehicleNo << "," << parentTaxiDriver->parent->getDatabaseId() << ","
            << currentNode->getNodeId() << "," << DailyTime(currentFleetItem.startTime*1000).getStrRepr() << "," << NULL << "," << NULL
            << "," << parentTaxiDriver->getDriverStatusStr() << std::endl;
    } else
    {
        out << currentFleetItem.vehicleNo << "," << parentTaxiDriver->parent->getDatabaseId() << ","
            << currentNode->getNodeId() << ","
            << (DailyTime(params.now.ms()) + DailyTime(ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr() << ","
            << (parentDriver->getParent()->getCurrSegStats()->getRoadSegment()->getRoadSegmentId()) << ","
            << ((parentDriver->getParent()->getCurrLane()) ? parentDriver->getParent()->getCurrLane()->getLaneId() : 0)
			<< "," << parentTaxiDriver->getDriverStatusStr()<< std::endl;
    }
	/* for Debug Purpose Only : to print details in Console
    	Print() << out.str();
    */
    taxitrajectoryLogger << out.str();
    return out.str();
}


bool TaxiDriverMovement::CruiseOnlyOrMoveToTaxiStand()
{
    return (random()%2);
}