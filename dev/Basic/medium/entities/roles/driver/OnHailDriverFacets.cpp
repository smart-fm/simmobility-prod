//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "entities/roles/passenger/Passenger.hpp"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "OnHailDriver.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnHailDriverMovement::OnHailDriverMovement() : currNode(nullptr), chosenTaxiStand(nullptr)
{
}

OnHailDriverMovement::~OnHailDriverMovement()
{
}

void OnHailDriverMovement::resetDriverLaneAndSegment()
{
	auto currSegStats = pathMover.getCurrSegStats();
	auto parent = onHailDriver->getParent();
	currLane = currSegStats->laneInfinity;
	parent->setCurrSegStats(currSegStats);
	parent->setCurrLane(currLane);

	onHailDriver->setToBeRemovedFromTaxiStand(true);
	onHailDriver->isExitingTaxiStand = true;

	auto vehicle = onHailDriver->getResource();
	vehicle->setMoving(true);
}

void OnHailDriverMovement::frame_init()
{
	//Create the vehicle and assign it to the role
	Vehicle *vehicle = new Vehicle(Vehicle::TAXI, TAXI_LENGTH);
	onHailDriver->setResource(vehicle);

	//Retrieve the starting node of the driver
	currNode = (*(onHailDriver->getParent()->currTripChainItem))->origin.node;

	//Make the behaviour decision
	BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

	//Perform the actions required based on the decision
	performDecisionActions(decision);

	onHailDriver->getParent()->setCurrSegStats(pathMover.getCurrSegStats());
}

void OnHailDriverMovement::frame_tick()
{
	switch (onHailDriver->getDriverStatus())
	{
	case CRUISING:
	{
		if(!onHailDriver->behaviour->isCruisingStintComplete() && !pathMover.isEndOfPath())
		{
			onHailDriver->behaviour->incrementCruisingStintTime();
		}
		else
		{
			//Make the behaviour decision
			BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

			//Perform the actions required based on the decision
			performDecisionActions(decision);
		}
		break;
	}
	case QUEUING_AT_TAXISTAND:
	{
		Person_MT *person = onHailDriver->tryTaxiStandPickUp();

		if(!person && !onHailDriver->behaviour->isQueuingStintComplete())
		{
			//No person picked up yet, but driver still has time to queue
			onHailDriver->behaviour->incrementQueuingStintTime();
		}
		else if(person)
		{
			try
			{
				//Person was picked up
				onHailDriver->addPassenger(person);
				beginDriveWithPassenger(person);

				//Driver must start from lane infinity at this point
				resetDriverLaneAndSegment();
			}
			catch (no_path_error &ex)
			{
				Warn() << ex.what();
				//What can be done in this case?
			}
		}
		else
		{
			//No person was picked up, but driver cannot queue any longer

			//Make the behaviour decision
			BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

			//Perform the actions required based on the decision
			performDecisionActions(decision);

			//Driver must start from lane infinity at this point
			resetDriverLaneAndSegment();
		}

		//Skip the multiple calls to frame_tick() from the conflux
		DriverUpdateParams &params = onHailDriver->getParams();
		params.elapsedSeconds = params.secondsInTick;
		onHailDriver->getParent()->setRemainingTimeThisTick(0.0);
		break;
	}
	case DRIVE_WITH_PASSENGER:
	{
		onHailDriver->passenger->Movement()->frame_tick();
		break;
	}
	}

	if(onHailDriver->getDriverStatus() != QUEUING_AT_TAXISTAND && !onHailDriver->isExitingTaxiStand)
	{
		DriverMovement::frame_tick();
	}

	//The job of this flag is done (we need it to skip only one call to DriverMovement::frame_tick), so reset it
	onHailDriver->isExitingTaxiStand = false;
}

string OnHailDriverMovement::frame_tick_output()
{
	return string();
}

bool OnHailDriverMovement::moveToNextSegment(DriverUpdateParams &params)
{
	//We can say that we're moving into a new link if we:
	//do not have a segment in the current link &
	//either there exists a segment in the next link or we're a the end of the current path
	bool isNewLinkNext = (!pathMover.hasNextSegStats(true) &&
			(pathMover.hasNextSegStats(false) || pathMover.isEndOfPath()));
	const SegmentStats *currSegStats = pathMover.getCurrSegStats();

	//Update the value of current node
	currNode = currSegStats->getRoadSegment()->getParentLink()->getFromNode();

	switch (onHailDriver->getDriverStatus())
	{
	case CRUISING:
	{
		//If we are moving from one link to another, try to pick up a person
		//that may be at the node
		if(isNewLinkNext)
		{
			const Link *currLink = currSegStats->getRoadSegment()->getParentLink();
			Person_MT *person = onHailDriver->tryPickUpPassengerAtNode(currLink->getToNode());

			if(person)
			{
				try
				{
					//Person was picked up
					onHailDriver->addPassenger(person);
					beginDriveWithPassenger(person);
				}
				catch (no_path_error &ex)
				{
					Warn() << ex.what();
					//What can be done in this case?
				}
			}
		}

		break;
	}
	case DRIVE_TO_TAXISTAND:
	{
		bool enteredTaxiStand = onHailDriver->tryEnterTaxiStand(currSegStats, chosenTaxiStand);

		if(enteredTaxiStand)
		{
			//Driver entered the taxi stand
			beginQueuingAtTaxiStand(params);
			return false;
		}

		break;
	}
	case DRIVE_WITH_PASSENGER:
	{
		if(pathMover.isEndOfPath())
		{
			//Arrived at the destination of the passenger
			onHailDriver->alightPassenger();
		}
		break;
	}
	}

	if(pathMover.isEndOfPath() && onHailDriver->getDriverStatus() != QUEUING_AT_TAXISTAND)
	{
		//Update the value of current node
		currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();

		//Driver has reached the path that was based on the previous task
		//Make the behaviour decision
		BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

		//Perform the actions required based on the decision
		performDecisionActions(decision);
	}

	return DriverMovement::moveToNextSegment(params);
}

void OnHailDriverMovement::performDecisionActions(BehaviourDecision decision)
{
	try
	{
		switch (decision)
		{
		case BehaviourDecision::CRUISE:
		{
			//Choose a node to cruise to
			const Node *node = onHailDriver->behaviour->chooseNode();

			//Begin cruising to the chosen node
			beginCruising(node);

			break;
		}
		case BehaviourDecision::DRIVE_TO_TAXISTAND:
		{
			//Choose a taxi stand to drive to
			chosenTaxiStand = onHailDriver->behaviour->chooseTaxiStand();

			//Begin driving toward chosen taxi stand
			beginDriveToTaxiStand(chosenTaxiStand);

			break;
		}
		case BehaviourDecision::END_SHIFT:
		{
			onHailDriver->getParent()->setToBeRemoved();
#ifndef NDEBUG
			ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
			                << onHailDriver->getParent()->getDatabaseId() << ": Shift ended"  << endl;
#endif
			break;
		}
		default:
		{
			stringstream msg;
			msg << "Unexpected decision:" << decision << " returned by method: makeBehaviourDecision()";
			throw runtime_error(msg.str());
		}
		}
	}
	catch(no_path_error &ex)
	{
		//Log the error
		Warn() << ex.what() << endl;

		//We try to cruise but to a different node this time.
		performDecisionActions(BehaviourDecision::CRUISE);
	}
}

void OnHailDriverMovement::beginDriveToTaxiStand(const TaxiStand *taxiStand)
{
	//Extract the taxi stand link and node
	const Link *taxiStandLink = taxiStand->getRoadSegment()->getParentLink();
	const Node *taxiStandNode = taxiStandLink->getToNode();

	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(taxiStandNode);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onHailDriver->getParent()->usesInSimulationTravelTime();

	//If the driving path has already been set, we must find path to the taxi stand from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	}

	//Get route to the taxi stand
	auto route = PrivateTrafficRouteChoice::getInstance()->getPathToLink(subTrip, false, currLink, nullptr,
	                                                                     taxiStandLink, useInSimulationTT);

#ifndef NDEBUG
	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
		    << " could not find a path to the taxi stand link " << taxiStandLink->getLinkId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}
#endif

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_TAXISTAND);
	onHailDriver->behaviour->resetQueuingStintTime();

	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving to taxi stand at link "
	                << taxiStandLink->getLinkId() << " from the current node " << currNode->getNodeId()
	                << " and link " << (currLink ? currLink->getLinkId() : 0) << endl;
}

void OnHailDriverMovement::beginCruising(const Node *node)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(node);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onHailDriver->getParent()->usesInSimulationTravelTime();

	//If the driving path has already been set, we must find path to the node from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	}

	//Get route to the node
	auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

#ifndef NDEBUG
	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
		    << " could not find a path to the cruising node " << node->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}
#endif

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
	onHailDriver->behaviour->resetCruisingStintTime();

	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin cruising from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to node " << node->getNodeId() << endl;
}

void OnHailDriverMovement::beginDriveWithPassenger(Person_MT *person)
{
	auto currSubTrip = person->currSubTrip;
	const Node *destination = (*currSubTrip).destination.node;
	const Link *currLink = nullptr;

	bool canDropPaxImmediately = false;

	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

		//If the drop-off node is at the end of the current link, we do not need to go anywhere
		//We can drop the passenger off at this point
		if(currLink->getToNode() == destination)
		{
			canDropPaxImmediately = true;
		}
	}
	else if(onHailDriver->getDriverStatus() == QUEUING_AT_TAXISTAND && currNode == destination)
	{
		//The passenger's destination is the same as the current node (which is the node at the end
		// of the taxi stand link)
		canDropPaxImmediately = true;
	}

	if(!canDropPaxImmediately)
	{
		//Create a sub-trip for the route choice
		SubTrip subTrip;
		subTrip.origin = WayPoint(currNode);
		subTrip.destination = WayPoint(destination);

		bool useInSimulationTT = onHailDriver->getParent()->usesInSimulationTravelTime();

		//Get route to the node
		auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

#ifndef NDEBUG
		if(route.empty())
		{
			stringstream msg;
			msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
			    << " could not find a path to the passenger's destination node " << destination->getNodeId()
			    << " from the current node " << currNode->getNodeId() << " and link ";
			msg << (currLink ? currLink->getLinkId() : 0);
			throw no_path_error(msg.str());
		}
#endif

		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(DRIVE_WITH_PASSENGER);

		ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
		                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving with pax from node "
		                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
		                << " to node " << destination->getNodeId() << endl;
	}
	else
	{
		//Person is already at the destination node
		onHailDriver->alightPassenger();

		//Make the behaviour decision
		BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

		//Perform the actions required based on the decision
		performDecisionActions(decision);
	}

	//Set vehicle to moving
	onHailDriver->getResource()->setMoving(true);
}

void OnHailDriverMovement::beginQueuingAtTaxiStand(DriverUpdateParams &params)
{
	auto vehicle = onHailDriver->getResource();

	if(vehicle->isMoving() && isQueuing)
	{
		removeFromQueue();
	}

	//Finalise the link travel time, as we will not be calling the DriverMovement::frame_tick()
	//where this normally happens
	const SegmentStats *currSegStat = pathMover.getCurrSegStats();
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	const Link *nextLink = RoadNetwork::getInstance()->getDownstreamLinks(currLink->getToNodeId()).front();
	auto parent = onHailDriver->getParent();

	double actualT = params.elapsedSeconds + params.now.ms() / 1000;
	parent->currLinkTravelStats.finalize(currLink, actualT, nextLink);
	TravelTimeManager::getInstance()->addTravelTime(parent->currLinkTravelStats); //in seconds
	currSegStat->getParentConflux()->setLinkTravelTimes(actualT, currLink);
	parent->currLinkTravelStats.reset();

	vehicle->setMoving(false);
	params.elapsedSeconds = params.secondsInTick;
	parent->setRemainingTimeThisTick(0.0);
	onHailDriver->setDriverStatus(QUEUING_AT_TAXISTAND);
	onHailDriver->setToBeRemovedFromTaxiStand(false);

	//Update the value of current node as we return after this method
	currNode = currLink->getToNode();

	//Clear the previous path. We will begin from the node
	pathMover.eraseFullPath();
	currLane = nullptr;

	ControllerLog() << parent->currTick.ms() << "ms: OnHailDriver "
	                << parent->getDatabaseId() << ": Begin queueing at taxi stand at segment "
	                << chosenTaxiStand->getRoadSegmentId() << endl;
}

BehaviourDecision OnHailDriverBehaviour::makeBehaviourDecision() const
{
	if(!hasDriverShiftEnded())
	{
		return (BehaviourDecision) Utils::generateInt((int) BehaviourDecision::CRUISE,
		                                              (int) BehaviourDecision::DRIVE_TO_TAXISTAND);
	}
	else
	{
		return BehaviourDecision ::END_SHIFT;
	}
}

const TaxiStand* OnHailDriverBehaviour::chooseTaxiStand() const
{
	const TaxiStand *result = nullptr;
	auto taxiStandsMap = RoadNetwork::getInstance()->getMapOfIdvsTaxiStands();
	auto itRandomStand = taxiStandsMap.begin();
	advance(itRandomStand,Utils::generateInt(0, taxiStandsMap.size() - 1));

	result = itRandomStand->second;

	//Ensure that we are not on the link which has the chosen taxi stand
	const MesoPathMover &pathMover = onHailDriver->movement->getMesoPathMover();
	if(pathMover.isDrivingPathSet() &&
			result->getRoadSegment()->getParentLink() == pathMover.getCurrSegStats()->getRoadSegment()->getParentLink())
	{
		result = chooseTaxiStand();
	}

	return result;
}

const Node* OnHailDriverBehaviour::chooseNode() const
{
	const Node *result = nullptr;
	auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	auto itRandomNode = nodeMap.begin();
	advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

	result = itRandomNode->second;

	//Ensure chosen node is not our immediate downstream node
	const MesoPathMover &pathMover = onHailDriver->movement->getMesoPathMover();
	if(pathMover.isDrivingPathSet() &&
			result == pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getToNode())
	{
		result = chooseNode();
	}

	return result;
}

bool OnHailDriverBehaviour::hasDriverShiftEnded() const
{
	return (onHailDriver->getParent()->currTick.ms() / 1000) >= onHailDriver->getParent()->getServiceVehicle().endTime;
}

void OnHailDriverBehaviour::incrementCruisingStintTime()
{
	currCruisingStintTime += onHailDriver->getParams().secondsInTick;
}

void OnHailDriverBehaviour::incrementQueuingStintTime()
{
	currQueuingStintTime += onHailDriver->getParams().secondsInTick;
}
