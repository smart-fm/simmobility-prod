/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * \file WaitBusActivity.hpp
 *
 * \author Yao Jin
 */

#pragma once

#include "entities/roles/Role.hpp"
#include "entities/UpdateParams.hpp"

//TODO: You can't use ../short for this:
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/activityRole/WaitBusActivityRole.hpp"
//#include "entities/roles/passenger/Passenger.hpp"

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
	WaitBusActivityRoleImpl(Agent* parent);
	virtual ~WaitBusActivityRoleImpl();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	//Virtual overrides
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);
	virtual UpdateParams& make_frame_tick_params(timeslice now);
};
}
