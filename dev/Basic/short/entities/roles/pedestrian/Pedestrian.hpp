//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/Role.hpp"
#include "PedestrianFacets.hpp"

namespace sim_mob
{

class Person_ST;

class Pedestrian : public Role<Person_ST>
{
private:
public:
	Pedestrian(Person_ST *parent, PedestrianBehaviour *behaviour = NULL, PedestrianMovement *movement = NULL,
			 Role<Person_ST>::Type roleType_ = Role<Person_ST>::RL_PEDESTRIAN, std::string roleName = "pedestrian");
	
	virtual ~Pedestrian();
	
	virtual Role<Person_ST>* clone(Person_ST *parent) const;

	virtual void make_frame_tick_params(timeslice now);

	virtual std::vector<BufferedBase *> getSubscriptionParams();
	
	/**
	 * Collect travel time for current role
	 */
	virtual void collectTravelTime();

	friend class PedestrianBehaviour;
	friend class PedestrianMovement;
};

}