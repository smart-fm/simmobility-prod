//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include <path/PathSetManager.hpp>
#include "geospatial/network/RoadNetwork.hpp"
#include "OnHailDriver.hpp"
#include "OnHailDriverFacets.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnHailDriverMovement::OnHailDriverMovement()
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
	BehaviourDecision decison = onHailDriver->behaviour->makeBehaviourDecision();

	//Perform the actions required based on the decision
	performDecisionActions(decison);
}

void OnHailDriverMovement::frame_tick()
{

}

string OnHailDriverMovement::frame_tick_output()
{
}

void OnHailDriverMovement::performDecisionActions(BehaviourDecision decision)
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
		const TaxiStand *taxiStand = onHailDriver->behaviour->chooseTaxiStand();

		//Begin driving toward chosen taxi stand
		beginDriveToTaxiStand(taxiStand);

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

	//If the driving path has already been set, we must find to the taxi stand from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	}

	//Get route to the taxi stand
	auto route = PrivateTrafficRouteChoice::getInstance()->getPathWhereToStand(subTrip, false, currLink, nullptr,
	                                                                           taxiStandLink, useInSimulationTT);

	if(!route.empty())
	{
		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_TO_TAXISTAND);
	}
	else
	{
		performDecisionActions(BehaviourDecision::CRUISE);
	}
}

void OnHailDriverMovement::beginCruising(const Node *node)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(node);

	const Link *currLink = nullptr;
	bool useInSimulationTT = onHailDriver->getParent()->usesInSimulationTravelTime();

	//If the driving path has already been set, we must find to the taxi stand from
	//the current segment
	if(pathMover.isDrivingPathSet())
	{
		currLink = pathMover.getCurrSegStats()->getRoadSegment()->getParentLink();
	}

	//Get route to the node
	auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

	if(!route.empty())
	{
		std::vector<const SegmentStats *> routeSegStats;
		pathMover.buildSegStatsPath(route, routeSegStats);
		pathMover.resetPath(routeSegStats);
		onHailDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
	}
	else
	{
		performDecisionActions(BehaviourDecision::CRUISE);
	}
}

BehaviourDecision OnHailDriverBehaviour::makeBehaviourDecision()
{
	return (BehaviourDecision) Utils::generateInt((int) BehaviourDecision::CRUISE,
	                                                        (int) BehaviourDecision::DRIVE_TO_TAXISTAND);
}

const TaxiStand* OnHailDriverBehaviour::chooseTaxiStand()
{
	auto taxiStandsMap = RoadNetwork::getInstance()->getMapOfIdvsTaxiStands();
	auto itRandomStand = taxiStandsMap.begin();
	advance(itRandomStand,Utils::generateInt(0, taxiStandsMap.size() - 1));

	return itRandomStand->second;
}

const Node* OnHailDriverBehaviour::chooseNode()
{
	auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	auto itRandomNode = nodeMap.begin();
	advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

	return itRandomNode->second;
}