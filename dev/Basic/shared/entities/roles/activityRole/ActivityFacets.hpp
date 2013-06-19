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
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

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
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p) {}

private:

	sim_mob::ActivityPerformer* parentActivity;
	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

	friend class ActivityPerformer;
};
}
