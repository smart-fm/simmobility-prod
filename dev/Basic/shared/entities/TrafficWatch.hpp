//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once


////////////////////////////////////////////////////////////
//
// TODO: This class was added by Zhemin to provide some statistics on the average speed
//       for a given segment.
//       He later commented this out, so the average speed is effectively not updated....
//       however, the StreetDirectory still calls "getAvgSpeedRS()" and attempts to
//       react to its value.
//       I've commented out the entire implementation (TrafficWatch.cpp), since it wasn't
//       being called anyway. This class either needs to be completely removed, or updated and
//       re-enabled in main.cpp.
//       One thing to think about: most items which "update" should be Agents. TrafficWatch
//       could be run as an Agent on a very large time-tick, depending on the required accuracy
//       of the data it sends. Or, we could just implement its "update" parameter in the
//       StreetDirectory itself, if a fine-grained resolution is required. Unfortunately,
//
//       Zhemin's commit message when he disabled this class is not helpful at all, so that is
//       why you are seeing this long "TODO" comment. ~Seth
//
////////////////////////////////////////////////////////////


#include <map>

#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "metrics/Frame.hpp"

namespace sim_mob
{

class RoadSegment;

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

	TrafficWatch() {}
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


	void update(timeslice now) {throw std::runtime_error("TrafficWatch not implemented");}




};
}
