//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "behavioral/params/WithindayModeParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"
#include "boost/unordered/unordered_map.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{
class DailyTime;

/**
 * Class to help build params required by withinday lua models
 *
 * \author Harish Loganathan
 */
class WithindayModelsHelper
{
private:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;

	/** map of zone code => zone params */
	static ZoneMap zoneMap;

	static bool initialized;

	const ZoneParams* findZone(int zoneCode) const;

public:
	WithindayModelsHelper();
	virtual ~WithindayModelsHelper();

	/**
	 * loads TAZ information from db and constructs zoneMap lookup
	 */
	static void loadZones();

	/**
	 * helper function to build parameters to pass to mode choice model
	 * @param curTrip current trip
	 * @param orgNd origin node for mode choice; (destination is taken from the trip)
	 * @param curTime time at which the withinday mode choice is to be called
	 * @return constructed withinday mode choice model params object
	 */
	WithindayModeParams buildModeChoiceParams(const Trip& curTrip, unsigned int orgNd, const DailyTime& curTime,const std::string& ptPathsetStoredProcName) const;
};
} //end namespace sim_mob
