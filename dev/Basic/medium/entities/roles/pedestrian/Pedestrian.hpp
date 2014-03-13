//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#pragma once

#include "entities/roles/Role.hpp"
#include "PedestrianFacets.hpp"

namespace sim_mob
{

class Agent;
class Person;

namespace medium
{

class PedestrianBehavior;
class PedestrianMovement;

/**
 * A medium-term Pedestrian.
 * \author Seth N. Hetu
 * \author zhang huai peng
 */
class Pedestrian : public sim_mob::Role {
public:
	int remainingTimeToComplete;

	explicit Pedestrian(Agent* parent, MutexStrategy mtxStrat, sim_mob::medium::PedestrianBehavior* behavior = nullptr, sim_mob::medium::PedestrianMovement* movement = nullptr);

	virtual ~Pedestrian() {}

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init() { throw std::runtime_error("frame_init not implemented in Pedestrian."); }
	virtual void frame_tick() { throw std::runtime_error("frame_tick not implemented in Pedestrian."); }
	virtual void frame_tick_output() { throw std::runtime_error("frame_tick_output not implemented in Pedestrian."); }
	virtual void make_frame_tick_params(timeslice now) { throw std::runtime_error("make_frame_tick_params not implemented in Pedestrian."); }
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams() { throw std::runtime_error("getSubscriptionParams not implemented in Pedestrian."); }

private:
	friend class PedestrainBehavior;
	friend class PedestrainMovement;
};


}}
