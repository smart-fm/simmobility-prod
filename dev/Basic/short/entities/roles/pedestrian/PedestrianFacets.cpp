//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "PedestrianFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Person_ST.hpp"
#include "conf/ConfigManager.hpp"

using namespace std;
using namespace sim_mob;

PedestrianMovement::PedestrianMovement() : 
MovementFacet(), parentPedestrian(nullptr), distanceToBeCovered(0)
{
}

PedestrianMovement::~PedestrianMovement()
{
}

void PedestrianMovement::frame_init()
{
	//Extract the origin and destination from the sub-trip
	
	SubTrip &subTrip = *(parentPedestrian->parent->currSubTrip);
	
	const Node *originNode = nullptr;
	const Point *origin = nullptr;
	const Point *destination = nullptr;
	
	switch(subTrip.origin.type)
	{
	case WayPoint::NODE: 
		originNode = subTrip.origin.node;
		origin = &(originNode->getLocation());
		break;
		
	case WayPoint::BUS_STOP:
		originNode = subTrip.origin.busStop->getParentSegment()->getParentLink()->getFromNode();
		origin = &(subTrip.origin.busStop->getStopLocation());
		break;
		
	case WayPoint::TRAIN_STOP:
		originNode = subTrip.origin.trainStop->getRandomStationSegment()->getParentLink()->getFromNode();
		origin = &(originNode->getLocation());
		break;
		
	default:
		stringstream msg;
		msg << __func__ << ": Origin type for pedestrian is invalid!\n";
		msg << "Type: " << subTrip.origin.type << " Person id: " << parentPedestrian->parent->getDatabaseId();
		throw runtime_error(msg.str());
	}
	
	switch(subTrip.destination.type)
	{
	case WayPoint::NODE: 
		destination = &(subTrip.destination.node->getLocation());
		break;
		
	case WayPoint::BUS_STOP:
		destination = &(subTrip.destination.busStop->getStopLocation());
		break;
		
	case WayPoint::TRAIN_STOP:
		destination = &(subTrip.destination.trainStop->getStationSegmentForNode(originNode)->getParentLink()->getToNode()->getLocation());
		break;
		
	default:
		stringstream msg;
		msg << __func__ << ": Destination type for pedestrian is invalid!\n";
		msg << "Type: " << subTrip.destination.type << " Person id: " << parentPedestrian->parent->getDatabaseId();
		throw runtime_error(msg.str());
	}
	
	//Get the distance between the origin and destination	
	DynamicVector distVector(*origin, *destination);
	distanceToBeCovered = distVector.getMagnitude();
	
	//Set the travel time in milli-seconds
	parentPedestrian->setTravelTime((distanceToBeCovered / parentPedestrian->parent->getWalkingSpeed()) * 1000);
}

void PedestrianMovement::frame_tick()
{
	double elapsedTime = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	double distanceCovered = parentPedestrian->parent->getWalkingSpeed() * elapsedTime;
	distanceToBeCovered -= distanceCovered;
	
	if(distanceToBeCovered <= 0)
	{
		parentPedestrian->getParent()->setToBeRemoved();
	}
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}


