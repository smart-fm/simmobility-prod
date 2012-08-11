/* Copyright Singapore-MIT Alliance for Research and Technology */


#pragma once

#include "entities/roles/Role.hpp"

namespace sim_mob
{

class Agent;
class Person;

namespace medium
{


/**
 * A medium-term Pedestrian.
 * \author Seth N. Hetu
 */
class Pedestrian : public sim_mob::Role {
public:
	int remainingTimeToComplete;

	Pedestrian(Agent* parent) {}
	virtual ~Pedestrian() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const {
		throw std::runtime_error("Not implemented.");
	}

	//Virtual overrides
	virtual void frame_init(UpdateParams& p) { throw std::runtime_error("Not implemented."); }
	virtual void frame_tick(UpdateParams& p) { throw std::runtime_error("Not implemented."); }
	virtual void frame_tick_output(const UpdateParams& p) { throw std::runtime_error("Not implemented."); }
	virtual void frame_tick_output_mpi(frame_t frameNumber) { throw std::runtime_error("Not implemented."); }
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS) { throw std::runtime_error("Not implemented."); }
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() { throw std::runtime_error("Not implemented."); }

private:

};


}}
