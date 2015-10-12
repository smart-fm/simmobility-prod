//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file ActivityPerformer.cpp
 *
 *  \author Harish
 */

#include "ActivityPerformer.hpp"

#include <cmath>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/Node.hpp"
#include "logging/Log.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::ActivityPerformer::ActivityPerformer(sim_mob::Person* parent, sim_mob::ActivityPerformerBehavior* behavior, sim_mob::ActivityPerformerMovement* movement, std::string roleName, Role::type roleType_):
		Role(behavior, movement, parent, roleName, roleType_), remainingTimeToComplete(0), location(nullptr)
{}

sim_mob::ActivityPerformer::ActivityPerformer(Person* parent, const sim_mob::Activity& currActivity, sim_mob::ActivityPerformerBehavior* behavior, sim_mob::ActivityPerformerMovement* movement, Role::type roleType_, std::string roleName) :
		Role(behavior, movement, parent, roleName, roleType_),remainingTimeToComplete(0), location(nullptr)
{
	activityStartTime = currActivity.startTime;
	activityEndTime = currActivity.endTime;
	location = currActivity.location;
}

sim_mob::ActivityPerformer::~ActivityPerformer() {}

Role* sim_mob::ActivityPerformer::clone(Person* parent) const
{
	ActivityPerformerBehavior* behavior = new ActivityPerformerBehavior(parent);
	ActivityPerformerMovement* movement = new ActivityPerformerMovement(parent);
	ActivityPerformer* activityRole = new ActivityPerformer(parent, behavior, movement, "activityRole");
	movement->parentActivity = activityRole;
	return activityRole;
}

std::vector<sim_mob::BufferedBase*> sim_mob::ActivityPerformer::getSubscriptionParams()
{
	return vector<BufferedBase*>();
}

sim_mob::DailyTime sim_mob::ActivityPerformer::getActivityEndTime() const
{
	return activityEndTime;
}

void sim_mob::ActivityPerformer::setActivityEndTime(sim_mob::DailyTime activityEndTime)
{
	this->activityEndTime = activityEndTime;
}

sim_mob::DailyTime sim_mob::ActivityPerformer::getActivityStartTime() const
{
	return activityStartTime;
}

void sim_mob::ActivityPerformer::setActivityStartTime(sim_mob::DailyTime activityStartTime)
{
	this->activityStartTime = activityStartTime;
}

const Node* sim_mob::ActivityPerformer::getLocation() const
{
	return location;
}

void sim_mob::ActivityPerformer::setLocation(const Node* location)
{
	this->location = location;
}

void sim_mob::ActivityPerformer::initializeRemainingTime()
{
	this->remainingTimeToComplete = this->activityEndTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime())
			- this->activityStartTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime());
}

int sim_mob::ActivityPerformer::getRemainingTimeToComplete() const
{
	return remainingTimeToComplete;
}

void sim_mob::ActivityPerformer::make_frame_tick_params(timeslice now){}

void sim_mob::ActivityPerformer::updateRemainingTime()
{
	this->remainingTimeToComplete = std::max(0, this->remainingTimeToComplete - int(ConfigManager::GetInstance().FullConfig().baseGranMS()));
}
