//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "OnCallDriverFacets.hpp"

#include "entities/controllers/MobilityServiceControllerManager.hpp"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "OnCallDriver.hpp"
#include "path/PathSetManager.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnCallDriverMovement::OnCallDriverMovement() : currNode(nullptr)
{
}

OnCallDriverMovement::~OnCallDriverMovement()
{
}

void OnCallDriverMovement::frame_init()
{
#ifndef NDEBUG
	if (!MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		throw std::runtime_error("No controller manager exists");
	}
#endif

	//Create the vehicle and assign it to the role
	Vehicle *vehicle = new Vehicle(Vehicle::TAXI, TAXI_LENGTH);
	onCallDriver->setResource(vehicle);

	//Retrieve the starting node of the driver
	currNode = (*(onCallDriver->getParent()->currTripChainItem))->origin.node;

	//Register with the controller to which the driver is subscribed
	auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();

	for(auto &ctrlr : controllers)
	{
		onCallDriver->subscribeToOrIgnoreController(controllers, ctrlr.second->getControllerId());
	}

	//In the beginning there is nothing to do, yet we require a path to begin moving.
	//So cruise to a random node, by creating a default schedule
	continueCruising();

	//Begin performing schedule.
	performScheduleItem();

	onCallDriver->getParent()->setCurrSegStats(pathMover.getCurrSegStats());

	onCallDriver->sendWakeUpShiftEndMsg();
}

void OnCallDriverMovement::frame_tick()
{
	switch (onCallDriver->getDriverStatus())
	{
	case CRUISING:
	{
		if(pathMover.isEndOfPath())
		{
			continueCruising();
			performScheduleItem();
		}
		break;
	}
	case PARKED:
	{
		//Skip the multiple calls to frame_tick() from the conflux
		DriverUpdateParams &params = onCallDriver->getParams();
		params.elapsedSeconds = params.secondsInTick;
		onCallDriver->getParent()->setRemainingTimeThisTick(0.0);
		break;
	}
	}

	if(onCallDriver->getDriverStatus() != PARKED)
	{
		DriverMovement::frame_tick();
	}
}

std::string OnCallDriverMovement::frame_tick_output()
{
	return std::string();
}

bool OnCallDriverMovement::moveToNextSegment(DriverUpdateParams &params)
{
	//Update the value of current node
	currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();

	if(pathMover.isEndOfPath())
	{
		switch (onCallDriver->getDriverStatus())
		{
		case CRUISING:
		{
			continueCruising();
			performScheduleItem();
			break;
		}
		case DRIVE_ON_CALL:
		{
			//Reached pick up node, pick-up passenger and perform next schedule item
			//Note: OnCallDriver::pickupPassenger marks the schedule item complete and moves to
			//the next item
			onCallDriver->pickupPassenger();
			performScheduleItem();
			break;
		}
		case DRIVE_WITH_PASSENGER:
		{
			//Reached destination, drop-off passenger and perform next schedule item
			//Note: OnCallDriver::pickupPassenger marks the schedule item complete and moves to
			//the next item
			onCallDriver->dropoffPassenger();
			performScheduleItem();
			break;
		}
		case DRIVE_TO_PARKING:
		{
			//Reached the parking location. Remove vehicle from the road
			parkVehicle(params);
			return false;
		}
		}
	}

	bool retVal = false;

	if(onCallDriver->getDriverStatus() != PARKED)
	{
		 retVal = DriverMovement::moveToNextSegment(params);
	}

	return retVal;
}

void OnCallDriverMovement::performScheduleItem()
{
	try
	{
		//Store the previous status
		MobilityServiceDriverStatus prevStatus = onCallDriver->getDriverStatus();

		if(onCallDriver->driverSchedule.isScheduleCompleted())
		{
			continueCruising();
		}

		//Get the current schedule item
		auto itScheduleItem = onCallDriver->driverSchedule.getCurrScheduleItem();
		bool hasShiftEnded = onCallDriver->behaviour->hasDriverShiftEnded();

		switch(itScheduleItem->scheduleItemType)
		{
		case CRUISE:
		{
			beginCruising(itScheduleItem->nodeToCruiseTo);

			//We need to call the beginCruising method above even if the shift has ended,
			//this is to allow the driver to notify the controller and receive a unsubscribe successful
			//reply
			if(hasShiftEnded && !onCallDriver->isWaitingForUnsubscribeAck)
			{
				onCallDriver->endShift();
			}
			break;
		}
		case PICKUP:
		{
			//Drive to pick up point
			beginDriveToPickUpPoint(itScheduleItem->tripRequest.startNode);
			break;
		}
		case DROPOFF:
		{
			//Drive to drop off point
			beginDriveToDropOffPoint(itScheduleItem->tripRequest.destinationNode);
			break;
		}
		case PARK:
		{
			//Drive to parking node
			beginDriveToParkingNode(itScheduleItem->parking->getAccessNode());
			break;
		}
		}

		//Reload the driver onto the network if it was in the parking
		if(prevStatus == PARKED)
		{
			onCallDriver->reload();
		}
	}
	catch(no_path_error &ex)
	{
		//Log the error
		Warn() << ex.what() << endl;

		//We need to recover from this error. Let controller know of the error and get another
		//schedule?
	}
}

void OnCallDriverMovement::beginCruising(const Node *node)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(node);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

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
		msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
		    << " could not find a path to the cruising node " << node->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
	                << onCallDriver->getParent()->getDatabaseId() << ": Begin cruising from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to node " << node->getNodeId() << endl;
#endif

	vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
}

void OnCallDriverMovement::beginDriveToPickUpPoint(const Node *pickupNode)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(pickupNode);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

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
		msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
		    << " could not find a path to the pickup node " << pickupNode->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}
#endif

	vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
	onCallDriver->sendScheduleAckMessage(true);

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
	                << onCallDriver->getParent()->getDatabaseId() << ": Begin driving from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to pickup node " << pickupNode->getNodeId() << endl;

	//Set vehicle to moving
	onCallDriver->getResource()->setMoving(true);
}

void OnCallDriverMovement::beginDriveToDropOffPoint(const Node *dropOffNode)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(dropOffNode);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

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
		msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
		    << " could not find a path to the drop off node " << dropOffNode->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
	                << onCallDriver->getParent()->getDatabaseId() << ": Begin driving with passenger from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to drop off node " << dropOffNode->getNodeId() << endl;
#endif

	vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);
}

void OnCallDriverMovement::beginDriveToParkingNode(const Node *parkingNode)
{
	//We call this method when at the end of a link. Hence, we should have a current link
	//and we'd be crossing into the next link soon.
	//If the 'toNode' for the link is the parking node, we can simply park the vehicle,
	//as we're about to move into the next link
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

	if(currLink->getToNode() != parkingNode)
	{
		//Create a sub-trip for the route choice
		SubTrip subTrip;
		subTrip.origin = WayPoint(currNode);
		subTrip.destination = WayPoint(parkingNode);
		bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

		//Get route to the node
		auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

#ifndef NDEBUG
		if (route.empty())
		{
			stringstream msg;
			msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
			    << " could not find a path to the parking node " << parkingNode->getNodeId()
			    << " from the current node " << currNode->getNodeId() << " and link ";
			msg << (currLink ? currLink->getLinkId() : 0);
			throw no_path_error(msg.str());
		}

		ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
		                << onCallDriver->getParent()->getDatabaseId() << ": Begin driving to park, from node "
		                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
		                << " to parking node " << parkingNode->getNodeId() << endl;
#endif

		vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_PARKING);
	}
	else
	{
		parkVehicle(onCallDriver->getParams());
	}
}

void OnCallDriverMovement::continueCruising()
{
	//Cruise to a random node, by creating a default schedule
	ScheduleItem cruise(CRUISE, onCallDriver->behaviour->chooseDownstreamNode(currNode));
	Schedule schedule;
	schedule.push_back(cruise);
	onCallDriver->driverSchedule.setSchedule(schedule);
}

void OnCallDriverMovement::parkVehicle(DriverUpdateParams &params)
{
	auto vehicle = onCallDriver->getResource();

	if(vehicle->isMoving() && isQueuing)
	{
		removeFromQueue();
	}

	//Finalise the link travel time, as we will not be calling the DriverMovement::frame_tick()
	//where this normally happens
	const SegmentStats *currSegStat = pathMover.getCurrSegStats();
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	const Link *nextLink = RoadNetwork::getInstance()->getDownstreamLinks(currLink->getToNodeId()).front();
	auto parent = onCallDriver->getParent();

	double actualT = params.elapsedSeconds + params.now.ms() / 1000;
	onCallDriver->getParent()->currLinkTravelStats.finalize(currLink, actualT, nextLink);
	TravelTimeManager::getInstance()->addTravelTime(parent->currLinkTravelStats); //in seconds
	currSegStat->getParentConflux()->setLinkTravelTimes(actualT, currLink);
	parent->currLinkTravelStats.reset();

	vehicle->setMoving(false);
	params.elapsedSeconds = params.secondsInTick;
	onCallDriver->getParent()->setRemainingTimeThisTick(0.0);

	//Update the value of current node as we return after this method
	currNode = onCallDriver->driverSchedule.getCurrScheduleItem()->parking->getAccessNode();

	onCallDriver->setDriverStatus(PARKED);
	onCallDriver->scheduleItemCompleted();

	//Clear the previous path. We will begin from the node
	pathMover.eraseFullPath();

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
	                << onCallDriver->getParent()->getDatabaseId() << ": Parked at node "
	                << currNode->getNodeId() << endl;
}

const Node * OnCallDriverBehaviour::chooseDownstreamNode(const Node *fromNode) const
{
	const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
	auto downstreamLinks = rdNetwork->getDownstreamLinks(fromNode->getNodeId());
	const MesoPathMover &pathMover = onCallDriver->movement->getMesoPathMover();
	const Lane *currLane = onCallDriver->movement->getCurrentlane();
	vector<const Node *> reachableNodes;

	//If we are continuing from an existing path, we need to check for connectivity
	//from the current lane
	if(pathMover.isDrivingPathSet() && currLane)
	{
		auto mapTurningsVsLanes = rdNetwork->getTurningPathsFromLanes();
		auto itTurningsFromCurrLane = mapTurningsVsLanes.find(currLane);

#ifndef NDEBUG
		if(itTurningsFromCurrLane == mapTurningsVsLanes.end())
		{
			stringstream msg;
			msg << "No downstream nodes are reachable from node " << fromNode->getNodeId();
			throw runtime_error(msg.str());
		}

		//Add all nodes that are reachable from the current lane to vector
		for(auto it = itTurningsFromCurrLane->second.begin(); it != itTurningsFromCurrLane->second.end(); ++it)
		{
			reachableNodes.push_back(it->second->getToLane()->getParentSegment()->getParentLink()->getToNode());
		}
#endif
	}
	else
	{
		//We are starting from a node and currently have no lane, all downstream nodes are
		//reachable
		for(auto link : downstreamLinks)
		{
			reachableNodes.push_back(link->getToNode());
		}
	}

#ifndef NDEBUG
	if(reachableNodes.empty())
	{
		stringstream msg;
		msg << "No downstream nodes are reachable from node " << fromNode->getNodeId()
		    << " and lane id " << (currLane ? currLane->getLaneId() : 0);
		throw runtime_error(msg.str());
	}
#endif

	//Select one node from the reachable nodes at random
	unsigned int random = Utils::generateInt(0, reachableNodes.size() - 1);
	return reachableNodes[random];
}

bool OnCallDriverBehaviour::hasDriverShiftEnded() const
{
	return (onCallDriver->getParent()->currTick.ms() / 1000) >= onCallDriver->getParent()->getServiceVehicle().endTime;
}