//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <conf/ConfigManager.hpp>
#include "OnCallDriverFacets.hpp"

#include "entities/controllers/MobilityServiceControllerManager.hpp"
#include "entities/ParkingAgent.hpp"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "OnCallDriver.hpp"
#include "path/PathSetManager.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

sim_mob::BasicLogger& onCallTrajectoryLogger  = sim_mob::Logger::log("onCall_taxi_trajectory.csv");

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
	onCallDriver->subscribeToController();

	//In the beginning there is nothing to do, yet we require a path to begin moving.
	//So cruise to a random node, by creating a default schedule
	continueCruising(currNode);

	//Begin performing schedule.
	performScheduleItem();

	onCallDriver->getParent()->setCurrSegStats(pathMover.getCurrSegStats());
	serviceVehicle = onCallDriver->getParent()->getServiceVehicle();

	onCallDriver->sendWakeUpShiftEndMsg();
}

void OnCallDriverMovement::frame_tick()
{
	if(onCallDriver->isScheduleUpdated)
	{
		performScheduleItem();

		//Reset the value
		onCallDriver->isScheduleUpdated = false;
	}

	switch (onCallDriver->getDriverStatus())
	{
	case CRUISING:
	{
		if(pathMover.isEndOfPath())
		{
			const Node *endOfPathNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getToNode();
			continueCruising(endOfPathNode);
			performScheduleItem();
		}
		break;
	}
	case DRIVE_WITH_PASSENGER:
	{
		//Call the passenger frame tick method to update the travel times
		for(auto itPax : onCallDriver->passengers)
		{
			itPax.second->Movement()->frame_tick();
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

	if(onCallDriver->getDriverStatus() != PARKED && !onCallDriver->isExitingParking)
	{
		DriverMovement::frame_tick();
	}

	//The job of this flag is done (we need it to skip only one call to DriverMovement::frame_tick), so reset it
	onCallDriver->isExitingParking = false;
}

std::string OnCallDriverMovement::frame_tick_output()
{
	const DriverUpdateParams &params = parentDriver->getParams();
	//if (pathMover.isPathCompleted() || ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	if (ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	std::ostringstream out(" ");

	if ((*(onCallDriver->getParent()->currTripChainItem))->origin.node == currNode && params.now.ms() == (uint32_t) 0)
	{
		out << getCurrentFleetItem().vehicleNo << "," << onCallDriver->getParent()->getDatabaseId() << ","
		<< currNode->getNodeId() << "," << DailyTime(serviceVehicle.startTime * 1000).getStrRepr()
		<< ",NULL,NULL," << onCallDriver->getDriverStatusStr()
		<< ", 0"
		<< ",No Passenger"
		<< std::endl;
	}
	else
	{

		//const Link * thisLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
		const std::string driverId = onCallDriver->getParent()->getDatabaseId();
		const unsigned int nodeId = currNode->getNodeId();
		const unsigned int roadSegmentId = (parentDriver->getParent()->getCurrSegStats()->getRoadSegment()->getRoadSegmentId());
		const Lane *currLane = parentDriver->getParent()->getCurrLane();
		const unsigned int currLaneId = (currLane ? parentDriver->getParent()->getCurrLane()->getLaneId() : 0);
		const std::string driverStatusStr = onCallDriver->getDriverStatusStr();
		const string timeStr = (DailyTime(params.now.ms()) + DailyTime(
				ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();

		out << getCurrentFleetItem().vehicleNo << "," << driverId << ","
		<< nodeId << ","
		<< timeStr << ","
		<< roadSegmentId << ","
		<< currLaneId
		<< "," << driverStatusStr
		<< ","<< onCallDriver->getPassengerCount()
		<<"," << onCallDriver->getPassengersId()
		<< std::endl;
	}
	/* for Debug Purpose Only : to print details in Console
    	Print() << out.str();
    */
	onCallTrajectoryLogger << out.str();
	return out.str();
}

bool OnCallDriverMovement::moveToNextSegment(DriverUpdateParams &params)
{
	//Update the value of current node
	const Link *currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	currNode = currLink->getFromNode();

	if(pathMover.isEndOfPath())
	{
		switch (onCallDriver->getDriverStatus())
		{
		case CRUISING:
		{
			continueCruising(currLink->getToNode());
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
		if(onCallDriver->driverSchedule.isScheduleCompleted())
		{
			onCallDriver->passengerInteractedDropOff=0;
			const Node *endOfPathNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getToNode();
			continueCruising(endOfPathNode);
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

			//We need to call the beginDriveToParkingNode method above even if the shift has ended,
			//this is to allow the driver to notify the controller and receive a unsubscribe successful
			//reply
			if(hasShiftEnded && !onCallDriver->isWaitingForUnsubscribeAck)
			{
				onCallDriver->endShift();
			}
			break;
		}
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

	//Get shortest path if path is not found in the path-set
	if(route.empty())
	{
		if(currLink)
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *node);
		}
		else
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *node);
		}
	}

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
#endif

	vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
	                << onCallDriver->getParent()->getDatabaseId() << ": Begin cruising from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to node " << node->getNodeId() << endl;
}

void OnCallDriverMovement::beginDriveToPickUpPoint(const Node *pickupNode)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(pickupNode);

	const Link *currLink = nullptr;
	Person_MT *parent = onCallDriver->getParent();
	bool useInSimulationTT = parent->usesInSimulationTravelTime();

	bool canPickPaxImmediately = false;

	//If the driving path has already been set, we must find path to the node from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

		//If the pickup node is at the end of the current link, we do not need to go anywhere
		//We can pick the passenger up at this point
		if(currLink->getToNode() == pickupNode)
		{
			canPickPaxImmediately = true;
            ControllerLog()<<"driver "<<parent->getDatabaseId()<<" is at same node "<<pickupNode->getNodeId()<<endl;
		}
	}
	else if(onCallDriver->getDriverStatus() == PARKED)
	{
		//Driver was parked and is now moving to pick-up node, so set this to true
		onCallDriver->isExitingParking = true;

		if(currNode == pickupNode)
		{
			//In case the driver was parked, if the pickup location is the same as that of
			//the parking node then we can pick up the passenger at this point
			canPickPaxImmediately = true;
            ControllerLog()<<"Driver "<<parent->getDatabaseId()<<" is at Parking and at same node "<<pickupNode->getNodeId()<<endl;
		}
	}

	if(canPickPaxImmediately)
	{
		onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
		onCallDriver->sendScheduleAckMessage(true);
        ControllerLog()<< "Driver will immediately pickup passenger."<<std::endl;
		onCallDriver->pickupPassenger();
		performScheduleItem();
		return;
	}

	//Get route to the node
	auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

	//Get shortest path if path is not found in the path-set
	if (route.empty())
	{
		if (currLink)
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *pickupNode);
		}
		else
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *pickupNode);
		}
	}

#ifndef NDEBUG
	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << parent->getDatabaseId()
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

    ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
                    << parent->getDatabaseId() << ": Begin driving with " << onCallDriver->getPassengerCount() <<" passenger(s) (" <<onCallDriver->getPassengersId()<< ") from node "
                    << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
                    << " to pickup node " << pickupNode->getNodeId()<<endl;

	//Set vehicle to moving
	onCallDriver->getResource()->setMoving(true);

	//If we're exiting a parking, currLane would be null. So set it to lane infinity
	if(onCallDriver->isExitingParking)
	{
		resetDriverLaneAndSegment();
	}
}

void OnCallDriverMovement::beginDriveToDropOffPoint(const Node *dropOffNode)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(dropOffNode);

	const Link *currLink = nullptr;
	Person_MT *parent = onCallDriver->getParent();
	bool useInSimulationTT = parent->usesInSimulationTravelTime();

	//If the driving path has already been set, we must find path to the node from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();

		//If the drop-off node is at the end of the current link, we do not need to go anywhere
		//We can drop the passenger off at this point
		if(currLink->getToNode() == dropOffNode)
		{
			onCallDriver->dropoffPassenger();
			performScheduleItem();
			return;
		}
	}

	//Get route to the node
	auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

	//Get shortest path if path is not found in the path-set
	if (route.empty())
	{
		if (currLink)
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *dropOffNode);
		}
		else
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *dropOffNode);
		}
	}

#ifndef NDEBUG
	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << parent->getDatabaseId()
		    << " could not find a path to the drop off node " << dropOffNode->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}
#endif

	vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);

	//Set vehicle to moving
	onCallDriver->getResource()->setMoving(true);

	ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
	                << parent->getDatabaseId() << ": Begin driving with " << onCallDriver->getPassengerCount() <<" passenger(s)(" <<onCallDriver->getPassengersId()<< ") from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to drop off node " << dropOffNode->getNodeId() <<endl;

	//If we're exiting a parking, this flag would be true. We need to set current lane to lane infinity
	//and also set the current segment based on the updated path
	//Note: This is possible as the pick-up could have been at the parking node itself
	if(onCallDriver->isExitingParking)
	{
		resetDriverLaneAndSegment();
	}
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

		//Get shortest path if path is not found in the path-set
		if (route.empty())
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *parkingNode);
		}

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
#endif

		vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_PARKING);

		ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
		                << onCallDriver->getParent()->getDatabaseId() << ": Begin driving with " << onCallDriver->getPassengerCount() <<" passenger(s) from node "
		                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
		                << " to parking node " << parkingNode->getNodeId() << endl;
	}
	else
	{
		parkVehicle(onCallDriver->getParams());
	}
}

void OnCallDriverMovement::continueCruising(const Node *fromNode)
{
	//Cruise to a random node, by creating a default schedule
	ScheduleItem cruise(CRUISE, onCallDriver->behaviour->chooseDownstreamNode(fromNode));
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
	parent->currLinkTravelStats.finalize(currLink, actualT, nextLink);
	TravelTimeManager::getInstance()->addTravelTime(parent->currLinkTravelStats); //in seconds
	currSegStat->getParentConflux()->setLinkTravelTimes(actualT, currLink);
	parent->currLinkTravelStats.reset();

	vehicle->setMoving(false);
	params.elapsedSeconds = params.secondsInTick;
	parent->setRemainingTimeThisTick(0.0);

	//Update the value of current node as we return after this method
	const SMSVehicleParking *parking = onCallDriver->driverSchedule.getCurrScheduleItem()->parking;
	currNode = parking->getAccessNode();

	//Add the person to the parking agent
	ParkingAgent *pkAgent = ParkingAgent::getParkingAgent(parking);
	pkAgent->addParkedPerson(parent);
	onCallDriver->setToBeRemovedFromParking(false);

	onCallDriver->setDriverStatus(PARKED);
	onCallDriver->scheduleItemCompleted();



	//Clear the previous path. We will begin from the node
	pathMover.eraseFullPath();
	currLane = nullptr;

	ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
	                << parent->getDatabaseId() << ": Parked at node "
	                << currNode->getNodeId() << endl;
}

void OnCallDriverMovement::resetDriverLaneAndSegment()
{
	auto currSegStats = pathMover.getCurrSegStats();
	auto parent = onCallDriver->getParent();
	currLane = currSegStats->laneInfinity;
	parent->setCurrSegStats(currSegStats);
	parent->setCurrLane(currLane);

	//Skip the multiple calls to frame_tick() from the conflux
	DriverUpdateParams &params = onCallDriver->getParams();
	params.elapsedSeconds = params.secondsInTick;
	onCallDriver->getParent()->setRemainingTimeThisTick(0.0);
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
#endif

		//Add all nodes that are reachable from the current lane to vector
		for(auto it = itTurningsFromCurrLane->second.begin(); it != itTurningsFromCurrLane->second.end(); ++it)
		{
			reachableNodes.push_back(it->second->getToLane()->getParentSegment()->getParentLink()->getToNode());
		}
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
	const Node *selectedNode = reachableNodes[random];

	//Check if we've selected a node which is the same as the fromNode
	//This can happen when there are small loops in the network and we will fail to get an
	//updated path
	while(selectedNode == fromNode)
	{
		//Choose a random node anywhere in the network
		selectedNode = chooseRandomNode();
	}

	return selectedNode;
}

const Node* OnCallDriverBehaviour::chooseRandomNode() const
{
	auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	auto itRandomNode = nodeMap.begin();
	advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

	const Node *result = itRandomNode->second;

	//Ensure chosen node is not a source/sink node
	if(result->getNodeType() == SOURCE_OR_SINK_NODE)
	{
		result = chooseRandomNode();
	}

	return result;
}

bool OnCallDriverBehaviour::hasDriverShiftEnded() const
{
	return (onCallDriver->getParent()->currTick.ms() / 1000) >= onCallDriver->getParent()->getServiceVehicle().endTime;
}