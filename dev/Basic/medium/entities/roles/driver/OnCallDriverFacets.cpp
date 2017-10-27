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
		onCallDriver->subscribeToOrIgnoreController(controllers, ctrlr.first);
	}

	//In the beginning there is nothing to do, yet we require a path to begin moving.
	//So cruise to a random node, by creating a default schedule
	continueCruising();

	onCallDriver->getParent()->setCurrSegStats(pathMover.getCurrSegStats());
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
	switch (onCallDriver->getDriverStatus())
	{
	case CRUISING:
	{
		if(pathMover.isEndOfPath())
		{
			continueCruising();
		}
		break;
	}
	}

	bool retVal = DriverMovement::moveToNextSegment(params);

	if(!pathMover.isPathCompleted())
	{
		//Update the value of current node
		currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();
	}

	return retVal;
}

void OnCallDriverMovement::performScheduleItem()
{
	try
	{
		//Get the first schedule item
		auto itScheduleItem = onCallDriver->driverSchedule.getCurrScheduleItem();
		bool hasShiftEnded = onCallDriver->behaviour->hasDriverShiftEnded();

		switch(itScheduleItem->scheduleItemType)
		{
		case CRUISE:
		{
			if(!hasShiftEnded)
			{
				beginCruising(itScheduleItem->nodeToCruiseTo);
			}
			else
			{
				onCallDriver->endShift();
			}
			break;
		}
		case PICKUP:
		{
			break;
		}
		case DROPOFF:
		{
			break;
		}
		case PARK:
		{
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

void OnCallDriverMovement::continueCruising()
{
	//Cruise to a random node, by creating a default schedule
	ScheduleItem cruise(CRUISE, onCallDriver->behaviour->chooseDownstreamNode(currNode));
	Schedule schedule;
	schedule.push_back(cruise);
	onCallDriver->driverSchedule.setSchedule(schedule);

	//Begin performing schedule.
	performScheduleItem();
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