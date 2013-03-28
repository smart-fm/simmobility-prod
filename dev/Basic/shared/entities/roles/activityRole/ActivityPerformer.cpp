/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file Pedestrian.cpp
 *
 *  \author Harish
 */

#include "ActivityPerformer.hpp"
#include "entities/Person.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent, std::string roleName) :
		Role(new sim_mob::ActivityPerformerBehavior(parent, rolename), new ActivityPerformerMovement(parent, rolename), parent,  roleName),
		params(parent->getGenerator()), remainingTimeToComplete(0), location(nullptr) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
}

sim_mob::ActivityPerformer::ActivityPerformer(Agent* parent, const sim_mob::Activity& currActivity, std::string roleName) :
		Role(new ActivityPerformerBehavior(parent, currActivity, roleName), new ActivityPerformerMovement(parent, currActivity, roleName), parent,  roleName),
		params(parent->getGenerator()), remainingTimeToComplete(0), location(nullptr) {
	//NOTE: Be aware that a null parent is certainly possible; what if we want to make a "generic" Pedestrian?
	//      The RoleManger in particular relies on this. ~Seth
	activityStartTime = currActivity.startTime;
	activityEndTime = currActivity.endTime;
	location = currActivity.location;
}

Role* sim_mob::ActivityPerformer::clone(Person* parent) const
{
	return new ActivityPerformer(parent);
}

sim_mob::ActivityPerformerUpdateParams::ActivityPerformerUpdateParams( boost::mt19937& gen) : UpdateParams(gen), skipThisFrame(true) {
}



std::vector<sim_mob::BufferedBase*> sim_mob::ActivityPerformer::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}
