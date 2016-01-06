/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"

#include <iterator>
#include <limits>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/Link.hpp"
#include "Pedestrian.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

PedestrianBehavior::PedestrianBehavior() : BehaviorFacet(), parentPedestrian(nullptr)
{
}

PedestrianBehavior::~PedestrianBehavior()
{
}

PedestrianMovement::PedestrianMovement(double speed) :
		MovementFacet(), parentPedestrian(nullptr), walkSpeed(speed), destinationNode(nullptr), totalTimeToCompleteSec(10),
		secondsInTick(ConfigManager::GetInstance().FullConfig().baseGranSecond())
{
}

PedestrianMovement::~PedestrianMovement()
{
}

void PedestrianMovement::setParentPedestrian(medium::Pedestrian* parentPedestrian)
{
	this->parentPedestrian = parentPedestrian;
}

TravelMetric& PedestrianMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& PedestrianMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}

void PedestrianBehavior::setParentPedestrian(medium::Pedestrian* parentPedestrian)
{
	this->parentPedestrian = parentPedestrian;
}

void PedestrianMovement::frame_init()
{
	destinationNode = getDestNode();
	if(!destinationNode)
	{
		throw std::runtime_error("destination segment not found");
	}

	SubTrip& subTrip = *(parentPedestrian->parent->currSubTrip);
	double walkTime = 0.0;
	if(subTrip.isPT_Walk)
	{
		walkTime = subTrip.walkTime; //walk time comes from db for PT pedestrians
	}
	else // both origin and destination must be nodes
	{
		if(subTrip.origin.type != WayPoint::NODE || subTrip.destination.type != WayPoint::NODE)
		{
			throw std::runtime_error("non node O/D for not PT pedestrian");
		}
		const Node* srcNode = subTrip.origin.node;
		const Node* destNode = subTrip.destination.node;

		DynamicVector distVector(srcNode->getLocation().getX(),srcNode->getLocation().getY(),destNode->getLocation().getX(),destNode->getLocation().getY());
		double distance = distVector.getMagnitude();
		walkTime = distance / walkSpeed;
	}
	parentPedestrian->setTravelTime(walkTime*1000);
}

const Node* PedestrianMovement::getDestNode()
{
	SubTrip& subTrip = *(parentPedestrian->parent->currSubTrip);
	const Node* destNd = nullptr;

	switch(subTrip.destination.type)
	{
	case WayPoint::NODE:
	{
		destNd = subTrip.destination.node;
		break;
	}
	case WayPoint::TRAIN_STOP:
	{
		const Node* srcNode = nullptr;
		switch(subTrip.origin.type)
		{
		case WayPoint::NODE:
		{
			srcNode = subTrip.origin.node;
			break;
		}
		case WayPoint::BUS_STOP:
		{
			srcNode = subTrip.origin.busStop->getParentSegment()->getParentLink()->getFromNode();
			break;
		}
		case WayPoint::TRAIN_STOP:
		{
			//this case should ideally not occur. handling just in case...
			srcNode = subTrip.origin.trainStop->getRandomStationSegment()->getParentLink()->getFromNode();
			break;
		}
		}
		destNd = subTrip.destination.trainStop->getStationSegmentForNode(srcNode)->getParentLink()->getToNode();
		break;
	}
	case WayPoint::BUS_STOP:
	{
		destNd = subTrip.destination.busStop->getParentSegment()->getParentLink()->getToNode();
		break;
	}
	}
	return destNd;
}

void PedestrianMovement::frame_tick()
{
	parentPedestrian->parent->setRemainingTimeThisTick(0);
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}

Conflux* PedestrianMovement::getDestinationConflux() const
{
	if (destinationNode)
	{
		return MT_Config::getInstance().getConfluxForNode(destinationNode);
	}
	return nullptr;
}
