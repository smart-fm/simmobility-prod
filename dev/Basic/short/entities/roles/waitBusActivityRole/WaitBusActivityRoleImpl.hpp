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
#include "entities/roles/waitBusActivityRole/WaitBusActivityRole.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRoleFacets.hpp"

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

class WaitBusActivityRoleImpl : public WaitBusActivityRole
{
public:
	WaitBusActivityRoleImpl(Person_ST *parent, WaitBusActivityRoleBehavior* behavior = nullptr, WaitBusActivityRoleMovement* movement = nullptr);
	virtual ~WaitBusActivityRoleImpl();

	virtual Role<Person_ST>* clone(Person_ST *parent) const;
};

class WaitBusActivityRoleBehaviorImpl : public WaitBusActivityRoleBehavior
{
public:
	WaitBusActivityRoleBehaviorImpl();
	virtual ~WaitBusActivityRoleBehaviorImpl();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
};

class WaitBusActivityRoleMovementImpl : public WaitBusActivityRoleMovement
{
public:
	WaitBusActivityRoleMovementImpl();
	virtual ~WaitBusActivityRoleMovementImpl();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
};
}
