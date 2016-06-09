//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_map.hpp>
#include "behavioral/params/WithindayModeParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"

namespace sim_mob
{
class Trip;
/**
 * Singleton class to help build params required by withinday lua models
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

	static void loadZones();

	const ZoneParams* findZone(int zoneCode) const;

public:
	WithindayModelsHelper();
	virtual ~WithindayModelsHelper();

	WithindayModeParams buildModeChoiceParams(const Trip& curTrip, unsigned int orgNd) const;
};
} //end namespace sim_mob
