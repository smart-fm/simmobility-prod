//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>

#include "entities/FleetController.hpp"

namespace sim_mob
{

class Entity;

namespace medium
{

class FleetController_MT : public FleetController
{
private:
	/**Static fleet manager object*/
	static FleetController_MT *fleetMgr;

	FleetController_MT();
	virtual ~FleetController_MT();

	/**
	 * Places the current agent into active or pending agents list based on the agent's start time
	 *
	 * @param person Pointer to the person object to be added to the list
	 * @param activeAgents The list of active agents
	 */
	void addOrStashTaxis(Person* person, std::set<Entity*>& activeAgents);

public:
	static FleetController_MT * getInstance();

	/**
	 *
	 * @param agentList
	 */
	virtual void initialise(std::set<sim_mob::Entity*>& agentList);
};

}
}