/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <vector>

#include "GenConfig.h"

#include "Agent.hpp"
#include "roles/Role.hpp"
#include "buffering/Shared.hpp"
#include "entities/UpdateParams.hpp"

namespace sim_mob
{


#ifndef SIMMOB_DISABLE_MPI
class PartitionManager;
class PackageUtils;
class UnPackageUtils;
#endif


class TrafficWatch {
private:

	static TrafficWatch instance_;
	static std::map<const RoadSegment*, double> avgSpeedRS;
	static std::map<const RoadSegment*, size_t> numVehRS;

	TrafficWatch() {} //Private constructor makes more sense for a singleton.
public:
    static TrafficWatch &
    instance()
    {
        return instance_;
    }
    std::map<const RoadSegment*, double> &
    getAvgSpeedRS()
    {
    	return avgSpeedRS;
    }

    std::map<const RoadSegment*, size_t> &
    getNumVehRS()
    {
    	return numVehRS;
    }


	void update(frame_t /* frameNumber */);




};
}
