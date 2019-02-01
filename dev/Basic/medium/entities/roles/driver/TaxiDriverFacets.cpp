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
sim_mob::BasicLogger& taxitrajectoryLogger  = sim_mob::Logger::log("taxi_trajectory.csv");


TaxiDriverMovement::TaxiDriverMovement()
{
}

TaxiDriverMovement::~TaxiDriverMovement()
{
}

void TaxiDriverMovement::frame_init()
{
#ifndef NDEBUG
	if (!MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		throw std::runtime_error("No controller manager exists");
	}
#endif

	Vehicle *newVeh = new Vehicle(Vehicle::TAXI, sim_mob::TAXI_LENGTH);
	parentTaxiDriver->setResource(newVeh);
	parentTaxiDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);

	assignFirstNode();

	serviceVehicle = parentTaxiDriver->getParent()->getServiceVehicle();

	/*const std::unordered_map<unsigned int , MobilityServiceController*>& controllers =
			MobilityServiceControllerManager::GetInstance()->getControllers();

	for (auto &p: controllers)
	{
		unsigned int controllerId = p.first;
		subscribeToOrIgnoreController(controllers, controllerId);
	}
	*/

	//jo{ Apr 11
	if (MT_Config::getInstance().isEnergyModelEnabled())
	{
		// INITIALIZE TRAJECTORY INFO AS EMPTY
		trajectoryInfo.totalDistanceDriven = 0.0;
		trajectoryInfo.totalTimeDriven = 0.0;
		trajectoryInfo.totalTimeFast = 0.0;
		trajectoryInfo.totalTimeSlow = 0.0;
	}
	// } jo

	(isSubscribedToOnHail() && cruiseOrDriveToTaxiStand())?driveToTaxiStand():selectNextLinkWhileCruising();  // for 1 : drive_to_taxiStand or cruise
}

void TaxiDriverMovement::subscribeToOrIgnoreController(
		const unordered_map<unsigned int , MobilityServiceController *> &controllers,
		unsigned int controllerId)
{
	if (serviceVehicle.controllerSubscription & controllerId)
	{
		auto range = controllers.find(controllerId);

#ifndef NDEBUG
		if (range == controllers.end())
		{
			std::stringstream msg;
			msg << "Driver " << parentDriver->getParent()->getDatabaseId() << " wants to subscribe to Id "
			    << (controllerId) << ", but no controller of that Id is registered";
			throw std::runtime_error(msg.str());
		}
#endif

		for (auto itController = range; itController != controllers.end(); ++itController)
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
			                << itController->second->toString() << " at time " << parentDriver->getParent()->currTick;
			ControllerLog() << ". parentDriver pointer " << parentDriver->getParent();

			ControllerLog() << std::endl;
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
		return nextSegStats->getInnermostLane();
	}
	else
	{
		const Lane *minLane = nullptr;
		double minQueueLength = std::numeric_limits<double>::max();
		double minLength = std::numeric_limits<double>::max();
		double que = 0.0;
		double total = 0.0;
		const Link *nextLink = getNextLinkForLaneChoice(nextSegStats);
		const std::vector<const sim_mob::Lane *> &lanes = nextSegStats->getRoadSegment()->getLanes();
		if (!minLane)
		{
			for (vector<const sim_mob::Lane *>::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
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
    const Link *link = currSegStat->getRoadSegment()->getParentLink();
    setCurrentNode(link->getFromNode());
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
			onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
			parentTaxiDriver->pickUpPassngerAtNode();
			// I cruise if there are no passengers
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
			onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
			selectNextLinkWhileCruising();
		}
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_WITH_PASSENGER && pathMover.isEndOfPath())
	{
		onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
		parentTaxiDriver->alightPassenger();
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_FOR_DRIVER_CHANGE_SHIFT && pathMover.isEndOfPath())
	{
		onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
		setCruisingMode();
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_FOR_BREAK && pathMover.isEndOfPath())
	{
		onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
		if (setBreakMode())
		{
			return res;
		}
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_ON_CALL && pathMover.isEndOfPath())
	{
		onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
		//Pick-up new passenger
		parentTaxiDriver->pickUpPassngerAtNode(personIdPickedUp);
	}
	else if (parentTaxiDriver->getDriverStatus() == DRIVE_TO_PARKING && pathMover.isEndOfPath())
    {
		onTripCompletion(); //jo Apr11 get Taxi Energy Computed from EnergyModelBase::onTaxiTripCompletion()
        parentTaxiDriver->setDriverStatus(PARKED);

        ControllerLog() << "Taxi driver " << parentTaxiDriver->getParent()->getDatabaseId() << " parked Taxi at time "
                        << parentTaxiDriver->getParent()->currTick
                        << " and at Node "
                        << destinationNode->getNodeId()
                        << std::endl;

		if (MobilityServiceControllerManager::HasMobilityServiceControllerManager() &&
				!parentTaxiDriver->hasDriverShiftEnded())
		{
			for (auto it = subscribedControllers.begin(); it != subscribedControllers.end(); ++it)
			{
				MessageBus::PostMessage(*it, MSG_DRIVER_AVAILABLE,
										MessageBus::MessagePtr(new DriverAvailableMessage(parentTaxiDriver->getParent())));
			}
		}

	    double actualT = params.elapsedSeconds + params.now.ms() / 1000;
	    const Link *nextLink = RoadNetwork::getInstance()->getDownstreamLinks(link->getToNodeId()).front();
	    parentTaxiDriver->getParent()->currLinkTravelStats.finalize(link, actualT, nextLink);
	    TravelTimeManager::getInstance()->addTravelTime(parentTaxiDriver->getParent()->currLinkTravelStats); //in seconds
	    currSegStat->getParentConflux()->setLinkTravelTimes(actualT, link);
	    parentTaxiDriver->getParent()->currLinkTravelStats.reset();

	    parentTaxiDriver->getResource()->setMoving(false);
	    params.elapsedSeconds = params.secondsInTick;
	    parentTaxiDriver->getParent()->setRemainingTimeThisTick(0.0);
	    currentNode = destinationNode;

	    currSegStat->getParentConflux()->getLinkStats(link).removeEntitiy(parentTaxiDriver->getParent());

	    parentTaxiDriver->processNextScheduleItem();

		return res;
    }

	if(parentTaxiDriver->getDriverStatus() != PARKED)
	{
		res = DriverMovement::moveToNextSegment(params);
	}

	return res;
}

bool TaxiDriverMovement::isSubscribedToOnHail() const
{
	std::map<unsigned int, MobilityServiceControllerConfig>::const_iterator it = ConfigManager::GetInstance().FullConfig().mobilityServiceController.enabledControllers.begin();
	MobilityServiceControllerType  type;
	while(it != ConfigManager::GetInstance().FullConfig().mobilityServiceController.enabledControllers.end())
	{
		if(serviceVehicle.controllerSubscription == it->first)
		{
			type = it->second.type;
			break;
		}
		it++;
	}
	if (type & MobilityServiceControllerType::SERVICE_CONTROLLER_ON_HAIL)
	{
		return true;
	}
	else if (type & MobilityServiceControllerType::SERVICE_CONTROLLER_UNKNOWN)
	{
		stringstream msg;
		msg << "Driver " << serviceVehicle.driverId << " subscribes to unknown service controller";
		throw runtime_error(msg.str());
	}
	else
	{
		return false;
	}
}

void TaxiDriverMovement::frame_tick()
{
	DriverUpdateParams &params = parentTaxiDriver->getParams();
    const SegmentStats *currSegStat = pathMover.getCurrSegStats();
    const Link *link = currSegStat->getRoadSegment()->getParentLink();
    setCurrentNode(link->getFromNode());
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();

	if (parentTaxiDriver->taxiPassenger != nullptr)
	{
		//Check the shift time has ended. If so, remove it from the simulation
		if(parentTaxiDriver->hasDriverShiftEnded() && isSubscribedToOnHail())
		{
			parentTaxiDriver->getParent()->setToBeRemoved();
		}

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
			(isSubscribedToOnHail() && cruiseOrDriveToTaxiStand())?driveToTaxiStand():setCruisingMode();  // for 1 : drive_to_taxiStand or cruise
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
		if (agent->isTaxiFirstInQueue(parentTaxiDriver->getParent()))
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
									addRouteChoicePath(currentRouteChoice);
									passenger->setStartPoint(WayPoint(destinationTaxiStand));
									passenger->setEndPoint(WayPoint(destinationNode));
									parentTaxiDriver->setDriverStatus(DRIVE_WITH_PASSENGER);
									parentTaxiDriver->getResource()->setMoving(true);
									toBeRemovedFromTaxiStand = true;
									previousTaxiStand = destinationTaxiStand;
									destinationTaxiStand = nullptr;
									passenger->setDriver(parentTaxiDriver);
									passenger->setStartPointDriverDistance(parentTaxiDriver->Movement()->getTravelMetric().distance);
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

    if (mode != QUEUING_AT_TAXISTAND && mode != DRIVER_IN_BREAK && mode != PARKED)
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
    setCurrentNode(link->getFromNode());
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
		currLane = firstSegStat->laneInfinity;
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

	const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
	auto downStreamLinks = rdNetwork->getDownstreamLinks(currentNode->getNodeId());
	auto linkItr = downStreamLinks.begin();
	int minNumOfVisits = -1;
	const Lane *currLane = getCurrentlane();

	auto turningPathsFromLanes = rdNetwork->getTurningPathsFromLanes();
	auto turningPathsLaneitr = turningPathsFromLanes.find(currLane);

	if (!currLane || turningPathsLaneitr != turningPathsFromLanes.end())
	{
		while (linkItr != downStreamLinks.end())
		{
			const Link *link = *linkItr;
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
				const std::vector<const sim_mob::Lane *> &segLanes = rdSegment->getLanes();
				bool foundTurningPath = false;
				for (std::vector<const sim_mob::Lane *>::const_iterator laneItr = segLanes.begin(); laneItr != segLanes.end();
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

const string TaxiDriverMovement::getPersonPickedUp() const
{
	return personIdPickedUp;
}

bool TaxiDriverMovement::driveToNodeOnCall(const TripRequestMessage &tripRequest, const Node *pickupNode)
{
	bool res = false;
	const MobilityServiceDriverStatus mode = parentTaxiDriver->getDriverStatus();
	if ((mode == CRUISING || mode == DRIVE_WITH_PASSENGER || mode == PARKED) && pickupNode)
	{
		const Link *link = nullptr;
		SubTrip currSubTrip;
		currSubTrip.destination = WayPoint(pickupNode);
		std::vector<WayPoint> currentRouteChoice;

		if(mode == PARKED ||
				(mode == DRIVE_WITH_PASSENGER && (currentNode == destinationNode && currentNode == originNode)))
		{
			//If we are leaving to pick a passenger up from parking itself, we do not need to consider our previous link
			//as we are at a node
			currSubTrip.origin = WayPoint(currentNode);
			currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, nullptr, true);
		}
		else
		{
			//We are on the link, so we need to consider a path with this link as the approach
			link = this->currLane->getParentSegment()->getParentLink();
			currSubTrip.origin = WayPoint(link->getToNode());
			currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, link, true);
		}


		if(currentRouteChoice.empty())
		{
			if(mode == PARKED)
			{
				currentRouteChoice = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currentNode,
				                                                                                       *pickupNode);
			}
			else
			{
				currentRouteChoice = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*link,
				                                                                                       *pickupNode);
			}
		}

		if (!currentRouteChoice.empty())
		{
			res = true;
			link = currentRouteChoice.front().link;
			currentNode = link->getFromNode();
			destinationNode = pickupNode;
			addRouteChoicePath(currentRouteChoice);
			parentTaxiDriver->setDriverStatus(DRIVE_ON_CALL);
			personIdPickedUp = tripRequest.userId;
		}
	}

	if (!res)
	{
		if (mode != CRUISING && mode != DRIVE_WITH_PASSENGER && mode !=PARKED)
		{
			ControllerLog() << "Assignment failed for " << tripRequest.userId
			                << " because mode was not CRUISING/DRIVE_WITH_PASSENGER/PARKED. Mode = " << mode
			                << std::endl;
		}
		else if (!pickupNode)
		{
			ControllerLog() << "Assignment failed for " << tripRequest.userId << " because pickup node was null" << std::endl;
		}
		else
		{
			ControllerLog() << "Assignment failed for " << tripRequest.userId << " because currentRouteChoice was empty"
			                << ". No path from lane " << this->currLane->getLaneId() << " to node "
			                << pickupNode->getNodeId() << std::endl;
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

		if(pathToNode.empty())
		{
			pathToNode = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *destination);
		}


		if (!pathToNode.empty())
		{
			result = true;
			currentNode = currLink->getFromNode();
			destinationNode = destination;
			addRouteChoicePath(pathToNode);
			parentTaxiDriver->setDriverStatus(CRUISING);
		}
		else
		{
			stringstream msg;
			msg << "Driver " << parentTaxiDriver->getParent()->getDatabaseId()
			    << " is unable to CRUISE to node " << destination->getNodeId() << ". No path available from "
			    << "current link " << currLink->getLinkId() << endl;
			throw runtime_error(msg.str());
		}
	}
	else if(destination == nullptr)
	{
		stringstream msg;
		msg << "Driver " << parentTaxiDriver->getParent()->getDatabaseId()
		    << " received invalid destination node. Unable to CRUISE to node.\n";
		throw runtime_error(msg.str());
	}
	else
	{
		stringstream msg;
		msg << "Driver " << parentTaxiDriver->getParent()->getDatabaseId()
		    << " is unable to CRUISE to node " << destination->getNodeId()
		    << ". Current mode is DRIVE_WITH_PASSENGER\n";
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
        const Node *destForRouteChoice = taxiStandLink->getToNode();
        //set origin and destination node
        SubTrip currSubTrip;
        currSubTrip.origin = WayPoint(ThisNode);
        currSubTrip.destination = WayPoint(destForRouteChoice);
        vector<WayPoint> routeToTaxiStand = PrivateTrafficRouteChoice::getInstance()->getPathToLink(currSubTrip,
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
	std::vector<const SegmentStats *> path;
	for (std::vector<WayPoint>::const_iterator itr = routeToDestination.begin(); itr != routeToDestination.end(); itr++)
	{
		if ((*itr).type == WayPoint::LINK)
		{
			const Link *link = (*itr).link;
			const Node *toNode = link->getToNode();
			Conflux *nodeConflux = Conflux::getConfluxFromNode(toNode);
			const std::vector<RoadSegment *> &roadSegments = link->getRoadSegments();
			for (std::vector<RoadSegment *>::const_iterator segItr = roadSegments.begin(); segItr != roadSegments.end();
			     segItr++)
			{
				const std::vector<SegmentStats *> &segStatsVector = nodeConflux->findSegStats(*segItr);
				path.insert(path.end(), segStatsVector.begin(), segStatsVector.end());
			}
		}
	}
	if (currentNode == originNode && pathMover.getPath().empty())
	{
		pathMover.setPath(path);
		const SegmentStats *firstSegStat = path.front();
		parentTaxiDriver->getParent()->setCurrSegStats(firstSegStat);
		parentTaxiDriver->getParent()->setCurrLane(firstSegStat->laneInfinity);
		currLane = firstSegStat->laneInfinity;
		isPathInitialized = true;
		pathMover.setSegmentStatIterator(firstSegStat);
	}
	else
	{
		const SegmentStats *currSegStat = pathMover.getCurrSegStats();
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
	const DriverUpdateParams &params = parentDriver->getParams();
	if (pathMover.isPathCompleted() || ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	std::ostringstream out(" ");

	if (originNode == currentNode && params.now.ms() == (uint32_t) 0)
	{
		out << serviceVehicle.vehicleNo << "," << parentTaxiDriver->parent->getDatabaseId() << ","
			<< currentNode->getNodeId() << "," << DailyTime(serviceVehicle.startTime * 1000).getStrRepr()
			<< ",NULL,NULL," << parentTaxiDriver->getDriverStatusStr()
			<< ",No Passenger"
			<< std::endl;
	}
	else
	{
		const std::string driverId = parentTaxiDriver->parent->getDatabaseId();
		const unsigned int nodeId = currentNode->getNodeId();
		const unsigned int roadSegmentId = (parentDriver->getParent()->getCurrSegStats()->getRoadSegment()->getRoadSegmentId());
		const Lane *currLane = parentDriver->getParent()->getCurrLane();
		const unsigned int currLaneId = (currLane ? parentDriver->getParent()->getCurrLane()->getLaneId() : 0);
		const std::string driverStatusStr = parentTaxiDriver->getDriverStatusStr();
		const string timeStr = (DailyTime(params.now.ms()) + DailyTime(
				ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();

		out << serviceVehicle.vehicleNo << "," << driverId << ","
			<< nodeId << ","
			<< timeStr << ","
			<< roadSegmentId << ","
			<< currLaneId
			<< "," << driverStatusStr
			<< ","<< parentTaxiDriver->getAllTaxiPassengersId()
			<< std::endl;
	}
	/* for Debug Purpose Only : to print details in Console
    	Print() << out.str();
    */
	taxitrajectoryLogger << out.str();
	return out.str();
}

bool TaxiDriverMovement::cruiseOrDriveToTaxiStand()
{
    return (random()%2);
}

bool TaxiDriverMovement::driveToParkingNode(const Node *destination)
{
	bool res = false;

	if (destination)
	{
		const Link *link = this->currLane->getParentSegment()->getParentLink();
		std::vector<WayPoint> currentRouteChoice = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(
				*link, *destination);

		if (currentRouteChoice.size() > 0)
		{
			res = true;
			currentNode = link->getFromNode();
			destinationNode = destination;
			addRouteChoicePath(currentRouteChoice);
			parentTaxiDriver->setDriverStatus(DRIVE_TO_PARKING);
		}
	}

	if (!res)
	{
		if (!destination)
		{
			ControllerLog() << "Taxi Driver " << parentTaxiDriver->getParent()->getDatabaseId()
			                << "can not go for parking because parking ID is not valid. " << std::endl;
		}
		else
		{
			ControllerLog() << "Parking Assignment failed for Taxi Driver "
			                << parentTaxiDriver->getParent()->getDatabaseId() << " because currentRouteChoice was empty"
			                << ". No path from lane " << this->currLane->getLaneId() << " to node "
			                << destination->getNodeId() << std::endl;
		}
	}

	return res;
}