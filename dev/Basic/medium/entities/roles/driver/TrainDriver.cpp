/*
 * TrainDriver.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: fm-simmobility
 */

#include <entities/roles/driver/TrainDriver.hpp>
#include "TrainDriverFacets.hpp"

namespace sim_mob {
namespace medium{

TrainDriver::TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior,
		sim_mob::medium::TrainMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
	sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType)

{

}

TrainDriver::~TrainDriver()
{

}

Role<Person_MT>* TrainDriver::clone(Person_MT *parent) const
{
	TrainBehavior* behavior = new TrainBehavior();
	TrainMovement* movement = new TrainMovement();
	TrainDriver* driver = new TrainDriver(parent, behavior, movement, "TrainDriver_");
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	return driver;
}

void TrainDriver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

std::vector<BufferedBase*> TrainDriver::getSubscriptionParams() {
	return std::vector<BufferedBase*>();
}
}
} /* namespace sim_mob */
