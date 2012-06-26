/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "entities/roles/Role.hpp"
#include "buffering/BufferedDataManager.hpp"

namespace sim_mob
{

/**
 * A Person in the Passenger role is likely just waiting for his or her bus stop.
 *
 * \author Luo Linbo
 * \author Seth N. Hetu
 * \author Xu Yan
 *
 *  \note
 *  This is a skeleton class. All functions are defined in this header file.
 *  When this class's full functionality is added, these header-defined functions should
 *  be moved into a separate cpp file.
 */
class Passenger : public sim_mob::Role {
public:
	virtual void update(frame_t frameNumber) { throw std::runtime_error("Passenger not yet implemented."); }

	//todo
	virtual void frame_init(UpdateParams& p) { throw std::runtime_error("Passenger not yet implemented."); }
	virtual void frame_tick(UpdateParams& p) { throw std::runtime_error("Passenger not yet implemented."); }
	virtual void frame_tick_output(const UpdateParams& p) { throw std::runtime_error("Passenger not yet implemented."); }
	virtual void frame_tick_output_mpi(frame_t frameNumber) {throw std::runtime_error("Passenger not yet implemented."); }
	virtual UpdateParams& make_frame_tick_params(frame_t frameNumber, unsigned int currTimeMS) { throw std::runtime_error("Passenger not yet implemented."); }

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams()
	{
		std::vector<sim_mob::BufferedBase*> res;
		return res;
	}

	//Serialization
#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil){}
	virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil){}
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){}
#endif

};



}
