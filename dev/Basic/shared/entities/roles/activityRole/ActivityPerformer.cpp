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
#include "geospatial/Node.hpp"
#include "logging/Log.hpp"

using std::vector;
using namespace std;
using namespace sim_mob;

ActivityPerformer::ActivityPerformer(Person* parent, ActivityPerformerBehavior* behavior, ActivityPerformerMovement* movement, string roleName, Type roleType_) 
: Role(parent, behavior, movement, roleName, roleType_), remainingTimeToComplete(0), location(nullptr)
{
}

ActivityPerformer::ActivityPerformer(Person* parent, const Activity& currActivity, ActivityPerformerBehavior* behavior, ActivityPerformerMovement* movement, 
									 Role::Type roleType_, string roleName) 
: Role(parent, behavior, movement, roleName, roleType_), remainingTimeToComplete(0), location(nullptr)
{
	activityStartTime = currActivity.startTime;
	activityEndTime = currActivity.endTime;
	location = currActivity.destination.node_;
}

sim_mob::ActivityPerformer::~ActivityPerformer()
{
}

Role* sim_mob::ActivityPerformer::clone(Person* parent) const
{
	ActivityPerformerBehavior* behavior = new ActivityPerformerBehavior();
	ActivityPerformerMovement* movement = new ActivityPerformerMovement();
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

sim_mob::Node* sim_mob::ActivityPerformer::getLocation() const
{
	return location;
}

void sim_mob::ActivityPerformer::setLocation(sim_mob::Node* location)
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

void sim_mob::ActivityPerformer::make_frame_tick_params(timeslice now)
{
}

void sim_mob::ActivityPerformer::updateRemainingTime()
{
	this->remainingTimeToComplete = std::max(0, this->remainingTimeToComplete - int(ConfigManager::GetInstance().FullConfig().baseGranMS()));
}
