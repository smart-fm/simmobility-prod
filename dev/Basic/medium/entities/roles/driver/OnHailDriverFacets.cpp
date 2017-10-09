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

OnHailDriverMovement::OnHailDriverMovement() : currNode(nullptr), chosenTaxiStand(nullptr)
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

		//Skip the multiple calls to frame_tick() from the conflux
		DriverUpdateParams &params = onHailDriver->getParams();
		params.elapsedSeconds = params.secondsInTick;
		onHailDriver->getParent()->setRemainingTimeThisTick(0.0);
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
	//We can say that we're moving into a new link if we:
	//do not have a segment in the current link &
	//either there exists a segment in the next link or we're a the end of the current path
	bool isNewLinkNext = (!pathMover.hasNextSegStats(true) &&
			(pathMover.hasNextSegStats(false) || pathMover.isEndOfPath()));
	const SegmentStats *currSegStats = pathMover.getCurrSegStats();

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

	bool retVal = DriverMovement::moveToNextSegment(params);

	//Update the value of current node
	currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();

	return retVal;
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

	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving to taxi stand at link "
	                << taxiStandLink->getLinkId() << " from the current node " << currNode->getNodeId()
	                << " and link " << (currLink ? currLink->getLinkId() : 0) << endl;
#endif

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_TAXISTAND);
	onHailDriver->behaviour->resetQueuingStintTime();
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
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

	//We call this method when at the end of a link or when at a taxi stand. In either case,
	//we should have a current link and we'd be crossing into the next link soon.
	//If the 'toNode' for the link is the destination for the person,
	//we can simply alight it, as we're about to move into the next link
	if(currLink->getToNode() != destination)
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

		ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
		                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving with pax from node "
		                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
		                << " to node " << destination->getNodeId() << endl;
#endif

		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(DRIVE_WITH_PASSENGER);
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

	vehicle->setMoving(false);
	params.elapsedSeconds = params.secondsInTick;
	onHailDriver->getParent()->setRemainingTimeThisTick(0.0);
	onHailDriver->setDriverStatus(QUEUING_AT_TAXISTAND);

	//Update the value of current node as we return after this method
	currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();
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
