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
class Pedestrian : public sim_mob::Role<Person_MT>
{
public:

	explicit Pedestrian(Person_MT* parent,
						sim_mob::medium::PedestrianBehavior* behavior = nullptr,
						sim_mob::medium::PedestrianMovement* movement = nullptr,
						std::string roleName = std::string("Pedestrian_"),
						Role<Person_MT>::Type roleType = Role<Person_MT>::RL_PEDESTRIAN);

	virtual ~Pedestrian();

	virtual sim_mob::Role<Person_MT>* clone(sim_mob::Person_MT *parent) const;

	virtual void make_frame_tick_params(timeslice now);

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	/**
	 * collect travel time for current role
	 */
	virtual void collectTravelTime();

private:
	friend class PedestrianBehavior;
	friend class PedestrianMovement;
};

}
}
