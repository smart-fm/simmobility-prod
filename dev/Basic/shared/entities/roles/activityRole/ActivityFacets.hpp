//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/RoleFacets.hpp"
#include "entities/UpdateParams.hpp"
#include "ActivityPerformer.hpp"
#include "entities/Person.hpp"

namespace sim_mob {
class ActivityPerformer;

class ActivityPerformerBehavior : public sim_mob::BehaviorFacet {
public:
	explicit ActivityPerformerBehavior(sim_mob::Person* parentAgent = nullptr, sim_mob::ActivityPerformer* parentRole = nullptr, std::string roleName = std::string());
	virtual ~ActivityPerformerBehavior() {}

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

private:

	sim_mob::ActivityPerformer* parentActivity;
	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

	friend class ActivityPerformer;
};

class ActivityPerformerMovement : public sim_mob::MovementFacet {
public:
	explicit ActivityPerformerMovement(sim_mob::Person* parentAgent = nullptr, sim_mob::ActivityPerformer* parentRole = nullptr, std::string roleName = std::string());
	virtual ~ActivityPerformerMovement() {}

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p) {}

private:

	sim_mob::ActivityPerformer* parentActivity;
	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

	friend class ActivityPerformer;
};
}
