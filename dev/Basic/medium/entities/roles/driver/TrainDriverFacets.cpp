/*
 * TrainDriverFacets.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: fm-simmobility
 */

#include <entities/roles/driver/TrainDriverFacets.hpp>

namespace sim_mob {
namespace medium{
TrainBehavior::TrainBehavior():BehaviorFacet(),parentDriver(nullptr)
{

}
TrainBehavior::~TrainBehavior()
{

}
void TrainBehavior::frame_init()
{

}
void TrainBehavior::frame_tick()
{

}
std::string TrainBehavior::frame_tick_output()
{
	return std::string();
}
TrainDriver* TrainBehavior::getParentDriver() const
{
	return parentDriver;
}

void TrainBehavior::setParentDriver(TrainDriver* parentDriver)
{
	if (!parentDriver)
	{
		throw std::runtime_error("parentDriver cannot be NULL");
	}
	this->parentDriver = parentDriver;
}

TrainMovement::TrainMovement():MovementFacet(),parentDriver(nullptr)
{

}
TrainMovement::~TrainMovement()
{

}
TravelMetric& TrainMovement::startTravelTimeMetric()
{
	return  travelMetric;
}


TravelMetric& TrainMovement::finalizeTravelTimeMetric()
{
	return  travelMetric;
}
void TrainMovement::frame_init()
{

}
void TrainMovement::frame_tick()
{

}
std::string TrainMovement::frame_tick_output()
{
	return std::string();
}
TrainDriver* TrainMovement::getParentDriver() const
{
	return parentDriver;
}

void TrainMovement::setParentDriver(TrainDriver* parentDriver)
{
	if (!parentDriver)
	{
		throw std::runtime_error("parentDriver cannot be NULL");
	}
	this->parentDriver = parentDriver;
}
}
} /* namespace sim_mob */
