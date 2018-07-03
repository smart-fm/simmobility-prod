//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "entities/roles/passenger/Passenger.hpp"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "OnHailDriver.hpp"
#include "path/PathSetManager.hpp"
#include "config/MT_Config.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;



sim_mob::BasicLogger& onHailTaxiTrajectoryLogger  = sim_mob::Logger::log("onhail_taxi_trajectory.csv");

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
	serviceVehicle = onHailDriver->getParent()->getServiceVehicle();
}

void OnHailDriverMovement::frame_tick()
{
	switch (onHailDriver->getDriverStatus())
	{
	case CRUISING:
	{
        if(onHailDriver->getParent()->canMoveToNextSegment != Person_MT::NONE)
        {
            break;
        }
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
		bool isLeavingWithoutPax = false;

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
				Warn() << ex.what() << endl;

				//What can be done in this case?
				//Remove the passenger from the vehicle (not drop off, because we would be alighting it at
				//the pickup node and telling it that it has arrived at the destination)
				onHailDriver->evictPassenger();
				isLeavingWithoutPax = true;
			}
		}
		else
		{
			//No person was picked up, but driver cannot queue any longer
			isLeavingWithoutPax = true;
		}

		if(isLeavingWithoutPax)
		{
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
	const DriverUpdateParams &params = parentDriver->getParams();
	if (pathMover.isPathCompleted() || ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	std::ostringstream out(" ");


	if ((*(onHailDriver->getParent()->currTripChainItem))->origin.node == currNode && params.now.ms() == (uint32_t) 0)
	{
		out << serviceVehicle.vehicleNo << "," << onHailDriver->getParent()->getDatabaseId() << ","
		<< currNode->getNodeId() << "," << DailyTime(serviceVehicle.startTime * 1000).getStrRepr()
		<< ",NULL,NULL," << onHailDriver->getDriverStatusStr()
		<< ", 0"
		<< ",No Passenger"
		<< std::endl;
	}
	else
	{
		const std::string driverId = onHailDriver->getParent()->getDatabaseId();
		const unsigned int nodeId = currNode->getNodeId();
		const unsigned int roadSegmentId = (parentDriver->getParent()->getCurrSegStats()->getRoadSegment()->getRoadSegmentId());
		const Lane *currLane = parentDriver->getParent()->getCurrLane();
		const unsigned int currLaneId = (currLane ? parentDriver->getParent()->getCurrLane()->getLaneId() : 0);
		const std::string driverStatusStr = onHailDriver->getDriverStatusStr();
		const string timeStr = (DailyTime(params.now.ms()) + DailyTime(
				ConfigManager::GetInstance().FullConfig().simStartTime())).getStrRepr();

		out << serviceVehicle.vehicleNo << "," << driverId << ","
		<< nodeId << ","
		<< timeStr << ","
		<< roadSegmentId << ","
		<< currLaneId
		<< "," << driverStatusStr
		<< ","<< onHailDriver->getPassengerCount()
		<<"," << onHailDriver->getPassengerId()
		<< std::endl;
	}



	/* for Debug Purpose Only : to print details in Console
    	Print() << out.str();
    */
	onHailTaxiTrajectoryLogger << out.str();
	return out.str();
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
					//Remove the passenger from the vehicle (not drop off, because we would be alighting it at
					//the pickup node and telling it that it has arrived at the destination)
					onHailDriver->evictPassenger();

					//Make the behaviour decision
					BehaviourDecision decision = onHailDriver->behaviour->makeBehaviourDecision();

					//Perform the actions required based on the decision
					performDecisionActions(decision);

					//Driver must start from lane infinity at this point
					resetDriverLaneAndSegment();
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
	std::vector<WayPoint> route = {};
	if(onHailDriver->isDriverControllerStudyAreaEnabled())
	{
		route = PrivateTrafficRouteChoice::getInstance()->getPathToLink(subTrip, false, currLink, nullptr,
																		taxiStandLink, useInSimulationTT,true);

	}
	else
	{
		route = PrivateTrafficRouteChoice::getInstance()->getPathToLink(subTrip, false, currLink, nullptr,
																		taxiStandLink, useInSimulationTT);
	}

	//Get shortest path if path is not found in the path-set
	if(route.empty())
	{
		if(currLink)
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Link>(*currLink, *taxiStandLink);
		}
		else
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Link>(*currNode, *taxiStandLink);
		}
	}

	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
		    << " could not find a path to the taxi stand link " << taxiStandLink->getLinkId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_TAXISTAND);
	onHailDriver->behaviour->resetQueuingStintTime();

	// Commenting below logs from controller.log as now we have onHailTrajectory to monitor driver movement
	/*
	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving to taxi stand at link "
	                << taxiStandLink->getLinkId() << " from the current node " << currNode->getNodeId()
	                << " and link " << (currLink ? currLink->getLinkId() : 0) << endl;
    */
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

	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
		    << " could not find a path to the cruising node " << node->getNodeId()
		    << " from the current node " << currNode->getNodeId() << " and link ";
		msg << (currLink ? currLink->getLinkId() : 0);
		throw no_path_error(msg.str());
	}

	std::vector<const SegmentStats *> routeSegStats;
	pathMover.buildSegStatsPath(route, routeSegStats);
	pathMover.resetPath(routeSegStats);
	onHailDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
	onHailDriver->behaviour->resetCruisingStintTime();

	// Commenting below logs from controller.log as now we have onHailTrajectory to monitor driver movement
	/*
	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin cruising from node "
	                << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
	                << " to node " << node->getNodeId() << endl;
    */
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

		//Get shortest path if path is not found in the path-set
		if(route.empty())
		{
			route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *destination);
		}

		if(route.empty())
		{
			stringstream msg;
			msg << "Path not found. Driver " << onHailDriver->getParent()->getDatabaseId()
			    << " could not find a path to the passenger "<<person->getDatabaseId() <<" destination node " << destination->getNodeId()
			    << " from the current node " << currNode->getNodeId() << " and link ";
			msg << (currLink ? currLink->getLinkId() : 0);
			throw no_path_error(msg.str());
		}

		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(DRIVE_WITH_PASSENGER);

		ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
		                << onHailDriver->getParent()->getDatabaseId() << ": Begin driving with pax "<<person->getDatabaseId() <<" from node "
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

	//Finalise the link travel time. This is normally done by DriverMovement::frame_tick() when we end
	//a link, but since we will be entering the segment again and calling setOrigin when we exit the
	//stand, we will attempt to start travel time again and run into errors. So, to simplify things,
	//we just end this travel time here, and start a new one later (Although both will not be accurate, neither will
	//the one that includes the queuing time)
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
	currNode = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getFromNode();

	// Commenting below logs from controller.log as now we have onHailTrajectory to monitor driver movement
	/*
	ControllerLog() << onHailDriver->getParent()->currTick.ms() << "ms: OnHailDriver "
	                << onHailDriver->getParent()->getDatabaseId() << ": Begin queueing at taxi stand at segment "
	                << chosenTaxiStand->getRoadSegmentId() << endl;
    */
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
	////Ensure chosen node is not a source/sink node
	const MesoPathMover &pathMover = onHailDriver->movement->getMesoPathMover();
	if ((result->getNodeType() == SOURCE_OR_SINK_NODE) ||
	    (pathMover.isDrivingPathSet() &&
	     result == pathMover.getCurrSegStats()->getRoadSegment()->getParentLink()->getToNode()))
	{
		result = chooseNode();
	}

	return result;
}

bool OnHailDriverBehaviour::hasDriverShiftEnded() const
{
	return ((onHailDriver->getParent()->currTick.ms()+ConfigManager::GetInstance().FullConfig().simStartTime().getValue())/ 1000) >= onHailDriver->getParent()->getServiceVehicle().endTime;
}

void OnHailDriverBehaviour::incrementCruisingStintTime()
{
	currCruisingStintTime += onHailDriver->getParams().secondsInTick;
}

void OnHailDriverBehaviour::incrementQueuingStintTime()
{
	currQueuingStintTime += onHailDriver->getParams().secondsInTick;
}
