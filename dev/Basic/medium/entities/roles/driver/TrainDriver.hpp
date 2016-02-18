/*
 * TrainDriver.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: fm-simmobility
 */

#include "entities/Person_MT.hpp"
#include "entities/roles/Role.hpp"
#include "TrainUpdateParams.hpp"

namespace sim_mob {
namespace medium{
class TrainBehavior;
class TrainMovement;
class TrainDriver : public sim_mob::Role<Person_MT>, public UpdateWrapper<TrainUpdateParams> {
public:

	virtual ~TrainDriver();

	TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior = nullptr,
		sim_mob::medium::TrainMovement* movement = nullptr,
		std::string roleName = std::string(),
		Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TRAINDRIVER);

	virtual Role<Person_MT>* clone(Person_MT *parent) const;
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

private:
	friend class TrainBehavior;
	friend class TrainMovement;
};
}
} /* namespace sim_mob */


