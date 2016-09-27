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
	
	const Point *origin = nullptr;
	const Point *destination = nullptr;
	
	switch(subTrip.origin.type)
	{
	case WayPoint::NODE: 
		origin = &(subTrip.origin.node->getLocation());
		break;
		
	case WayPoint::BUS_STOP:
		origin = &(subTrip.origin.busStop->getStopLocation());
		break;
		
	default:
		stringstream msg;
		msg << "Origin type for pedestrian is invalid!\n";
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
		
	default:
		stringstream msg;
		msg << "Destination type for pedestrian is invalid!\n";
		msg << "Type: " << subTrip.destination.type << " Person id: " << parentPedestrian->parent->getDatabaseId();
		throw runtime_error(msg.str());
	}
	
	//Get the distance between the origin and destination
	
	DynamicVector distVector(*origin, *destination);
	distanceToBeCovered = distVector.getMagnitude();
}

void PedestrianMovement::frame_tick()
{
	if(distanceToBeCovered > 0)
	{
		double elapsedTime = ConfigManager::GetInstance().FullConfig().baseGranSecond();
		double distanceCovered = parentPedestrian->parent->getWalkingSpeed() * elapsedTime;
		distanceToBeCovered -= distanceCovered;
	}
	else
	{
		parentPedestrian->getParent()->setToBeRemoved();
	}
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}


