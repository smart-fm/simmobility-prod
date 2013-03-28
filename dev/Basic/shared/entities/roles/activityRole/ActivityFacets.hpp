#include "entities/roles/RoleFacets.hpp"
#include "entities/UpdateParams.hpp"
#include "ActivityPerformer.hpp"

namespace sim_mob {

class ActivityPerformerBehavior : public sim_mob::BehaviorFacet {
public:
	int remainingTimeToComplete;

	explicit ActivityPerformerBehavior(sim_mob::Agent* parentAgent = nullptr, sim_mob::ActivityPerformer* parentRole = nullptr, std::string roleName = std::string());
	virtual ~ActivityPerformerBehavior() {}

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	sim_mob::DailyTime getActivityEndTime() const;
	void setActivityEndTime(sim_mob::DailyTime activityEndTime);
	sim_mob::DailyTime getActivityStartTime() const;
	void setActivityStartTime(sim_mob::DailyTime activityStartTime);
	sim_mob::Node* getLocation() const;
	void setLocation(sim_mob::Node* location);
	void initializeRemainingTime();
	void updateRemainingTime();

private:
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
	sim_mob::Node* location;
	sim_mob::ActivityPerformer* parentActivity;

	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;

	friend class ActivityPerformer;
};

class ActivityPerformerMovement : public sim_mob::MovementFacet {
public:
	explicit ActivityPerformerMovement(sim_mob::Agent* parentAgent = nullptr, sim_mob::Role* parentRole = nullptr, std::string roleName = std::string());
	virtual ~ActivityPerformerMovement() {}

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

private:
	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;
};
}
