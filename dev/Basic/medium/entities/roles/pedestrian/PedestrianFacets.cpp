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
#include "geospatial/BusStop.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "Pedestrian.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
	/**
	 * gets a random upstream segment for a node
	 * @param node the node of interest
	 * @returns random upstream segment
	 */
	const RoadSegment* getRandomUpstreamSegAtNode(const Node* node)
	{
		const MultiNode* multiNd = dynamic_cast<const MultiNode*>(node);
		if(!multiNd)
		{
			const UniNode* uniNd = dynamic_cast<const UniNode*>(node);
			if(!uniNd)
			{
				throw std::runtime_error("not a node");
			}
			int random = Utils::generateInt(0, 1);
			if(random == 0)
			{
				return uniNd->firstPair.first; //first is a from section
			}
			else
			{
				return uniNd->secondPair.first; //first is a from section
			}
		}
		const std::set<RoadSegment*>& segmentsAtDest = multiNd->getRoadSegments();
		if(segmentsAtDest.empty()) { throw std::runtime_error("no segments at multinode"); }
		if(segmentsAtDest.size() == 1)
		{
			return *(segmentsAtDest.begin());
		}
		else
		{
			int random = Utils::generateInt(0, segmentsAtDest.size()-1);
			std::set<RoadSegment*>::const_iterator segIt = segmentsAtDest.begin();
			std::advance(segIt, random);
			return *(segIt);
		}
	}
}

namespace sim_mob
{
namespace medium
{

PedestrianBehavior::PedestrianBehavior() : BehaviorFacet(), parentPedestrian(nullptr)
{
}

PedestrianBehavior::~PedestrianBehavior()
{
}

PedestrianMovement::PedestrianMovement(double speed) :
		MovementFacet(), parentPedestrian(nullptr), remainingTimeToComplete(0), walkSpeed(speed), destinationSegment(nullptr), totalTimeToCompleteSec(10),
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
	destinationSegment = getDestSegment();
	if(!destinationSegment)
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
		if(subTrip.origin.type_ != WayPoint::NODE || subTrip.destination.type_ != WayPoint::NODE)
		{
			throw std::runtime_error("non node O/D for not PT pedestrian");
		}
		const Node* srcNode = subTrip.origin.node_;
		const Node* destNode = subTrip.destination.node_;

		DynamicVector distVector(srcNode->getLocation().getX(),srcNode->getLocation().getY(),destNode->getLocation().getX(),destNode->getLocation().getY());
		double distance = distVector.getMagnitude();
		walkTime = distance / walkSpeed;
	}
	remainingTimeToComplete = walkTime;
	parentPedestrian->setTravelTime(walkTime*1000);
}

const RoadSegment* PedestrianMovement::getDestSegment()
{
	SubTrip& subTrip = *(parentPedestrian->parent->currSubTrip);
	const RoadSegment* destSeg = nullptr;

	switch(subTrip.destination.type_)
	{
	case WayPoint::NODE:
	{
		destSeg = getRandomUpstreamSegAtNode(subTrip.destination.node_);
		break;
	}
	case WayPoint::MRT_STOP:
	{
		const Node* srcNode = nullptr;
		switch(subTrip.origin.type_)
		{
		case WayPoint::NODE:
		{
			srcNode = subTrip.origin.node_;
			break;
		}
		case WayPoint::BUS_STOP:
		{
			srcNode = subTrip.origin.busStop_->getParentSegment()->getStart();
			break;
		}
		case WayPoint::MRT_STOP:
		{
			//this case should ideally not occur. handling just in case...
			srcNode = subTrip.origin.mrtStop_->getRandomStationSegment()->getStart();
			break;
		}
		}
		destSeg = subTrip.destination.mrtStop_->getStationSegmentForNode(srcNode);
		break;
	}
	case WayPoint::BUS_STOP:
	{
		destSeg = subTrip.destination.busStop_->getParentSegment();
		break;
	}
	}
	return destSeg;
}

void PedestrianMovement::frame_tick()
{
	if (remainingTimeToComplete <= secondsInTick)
	{
		parentPedestrian->parent->setNextLinkRequired(nullptr);
		parentPedestrian->parent->setToBeRemoved();
	}
	else
	{
		remainingTimeToComplete -= secondsInTick;
	}
	parentPedestrian->parent->setRemainingTimeThisTick(0);
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}

Conflux* PedestrianMovement::getStartingConflux() const
{
	if (destinationSegment)
	{
		return Conflux::getConflux(destinationSegment);
	}
	return nullptr;
}

} /* namespace medium */
} /* namespace sim_mob */

