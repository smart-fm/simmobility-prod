//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "OnHailDriver.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnHailDriverMovement::OnHailDriverMovement() : currNode(nullptr), chosenTaxiStand(nullptr), isMovedIntoNextLink(false)
{
}

OnHailDriverMovement::~OnHailDriverMovement()
{
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
	const MobilityServiceDriverStatus status = onHailDriver->getDriverStatus();

	switch (status)
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
			//Person was picked up
			onHailDriver->addPassenger(person);
			beginDriveWithPassenger(person);
		}
		else
		{
			//No person was picked up, but driver cannot queue any longer

			//Make the behaviour decision
			BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

			//Perform the actions required based on the decision
			performDecisionActions(decision);
		}

		break;
	}
	}

	if(onHailDriver->getDriverStatus() != QUEUING_AT_TAXISTAND)
	{
		DriverMovement::frame_tick();
	}
}

string OnHailDriverMovement::frame_tick_output()
{
	return string();
}

bool OnHailDriverMovement::moveToNextSegment(DriverUpdateParams &params)
{
	const SegmentStats *currSegStats = pathMover.getCurrSegStats();

	switch (onHailDriver->getDriverStatus())
	{
	case CRUISING:
	{
		//If we are moving from one link to another, try to pick up a person
		//that may be at the node
		if(isMovedIntoNextLink)
		{
			Person_MT *person = onHailDriver->tryPickUpPassengerAtNode(currNode);

			if(person)
			{
				//Person was picked up
				onHailDriver->addPassenger(person);
				beginDriveWithPassenger(person);
			}

			isMovedIntoNextLink = false;
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
		//Driver has reached the node it was cruising to
		//Make the behaviour decision
		BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

		//Perform the actions required based on the decision
		performDecisionActions(decision);
	}

	return DriverMovement::moveToNextSegment(params);
}

void OnHailDriverMovement::flowIntoNextLinkIfPossible(DriverUpdateParams &params)
{
	//Store the link before attempting to move into the next link
	const Link *prevLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

	DriverMovement::flowIntoNextLinkIfPossible(params);

	//Get the current link
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

	if(prevLink != currLink)
	{
		//Moved into the next link, update the current node
		currNode = currLink->getFromNode();
		isMovedIntoNextLink = true;
	}
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

	ControllerLog() << "OnHailDriver " << onHailDriver->getParent()->getDatabaseId()
	                << ": Begin driving to taxi stand at link " << taxiStandLink->getLinkId() << endl;
#endif

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_TAXISTAND);
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

	ControllerLog() << "OnHailDriver " << onHailDriver->getParent()->getDatabaseId()
	                << ": Begin cruising to node " << node->getNodeId() << endl;
#endif

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
	onHailDriver->behaviour->resetCruisingStintTime();
}

void OnHailDriverMovement::beginDriveWithPassenger(Person_MT *person)
{
	auto currSubTrip = person->currSubTrip;
	const Node *destination = (*currSubTrip).destination.node;

	if(currNode != destination)
	{
		//Create a sub-trip for the route choice
		SubTrip subTrip;
		subTrip.origin = WayPoint(currNode);
		subTrip.destination = WayPoint(destination);

		const Link *currLink = nullptr;
		bool useInSimulationTT = onHailDriver->getParent()->usesInSimulationTravelTime();

		//If the driving path has already been set, we must find path to the destination node from
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
			    << " could not find a path to the passenger's destination node " << destination->getNodeId()
			    << " from the current node " << currNode->getNodeId() << " and link ";
			msg << (currLink ? currLink->getLinkId() : 0);
			throw no_path_error(msg.str());
		}

		ControllerLog() << "OnHailDriver " << onHailDriver->getParent()->getDatabaseId()
		                << ": Begin driving with pax to node " << destination->getNodeId() << endl;
#endif

		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);
		onHailDriver->behaviour->resetCruisingStintTime();
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
}

void OnHailDriverMovement::beginQueuingAtTaxiStand(DriverUpdateParams &params)
{
	auto vehicle = onHailDriver->getResource();

	if(vehicle->isMoving() && isQueuing)
	{
		removeFromQueue();
	}

	vehicle->setMoving(false);
	params.elapsedSeconds = params.secondsInTick;
	onHailDriver->getParent()->setRemainingTimeThisTick(0.0);
	onHailDriver->setDriverStatus(QUEUING_AT_TAXISTAND);
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
	auto taxiStandsMap = RoadNetwork::getInstance()->getMapOfIdvsTaxiStands();
	auto itRandomStand = taxiStandsMap.begin();
	advance(itRandomStand,Utils::generateInt(0, taxiStandsMap.size() - 1));

	return itRandomStand->second;
}

const Node* OnHailDriverBehaviour::chooseNode() const
{
	auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	auto itRandomNode = nodeMap.begin();
	advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

	return itRandomNode->second;
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
