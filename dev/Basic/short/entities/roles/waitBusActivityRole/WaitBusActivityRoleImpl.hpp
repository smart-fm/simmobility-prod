//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"

#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/activityRole/WaitBusActivityRole.hpp"
#include "entities/roles/activityRole/WaitBusActivityRoleFacets.hpp"

namespace sim_mob
{

class BusDriver;
class BusStopAgent;
class Passenger;
class PackageUtils;
class UnPackageUtils;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

class WaitBusActivityRoleImpl : public sim_mob::WaitBusActivityRole {
public:
	WaitBusActivityRoleImpl(Agent* parent, sim_mob::WaitBusActivityRoleBehavior* behavior = nullptr, sim_mob::WaitBusActivityRoleMovement* movement = nullptr);
	virtual ~WaitBusActivityRoleImpl();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
};

class WaitBusActivityRoleBehaviorImpl : public sim_mob::WaitBusActivityRoleBehavior {
public:
	WaitBusActivityRoleBehaviorImpl(sim_mob::Person* parentAgent = nullptr);
	virtual ~WaitBusActivityRoleBehaviorImpl();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
};

class WaitBusActivityRoleMovementImpl : public sim_mob::WaitBusActivityRoleMovement {
public:
	WaitBusActivityRoleMovementImpl(sim_mob::Person* parentAgent = nullptr);
	virtual ~WaitBusActivityRoleMovementImpl();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();
	virtual void flowIntoNextLinkIfPossible(UpdateParams& p);
};
}
