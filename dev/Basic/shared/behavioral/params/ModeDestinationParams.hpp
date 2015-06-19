//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ModeDestinationParams.hpp
 *
 *  Created on: Nov 30, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include <cmath>
#include <string>
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/StopType.hpp"

namespace sim_mob {

/**
 * Base class for tour and stop mode destination params
 */
class ModeDestinationParams {
protected:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;

	StopType purpose;
	int origin;
	const double OPERATIONAL_COST;
	const double MAX_WALKING_DISTANCE;
	const ZoneMap& zoneMap;
	const CostMap& amCostsMap;
	const CostMap& pmCostsMap;
	int cbdOrgZone;

public:
	ModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, StopType purpose, int originCode);
	virtual ~ModeDestinationParams();

	/**
	 * Returns the mode of a particular choice
	 *
	 * @param choice an integer ranging from 1 to (numZones*numModes = 9828) representing a unique combination of mode and destination
	 * 			(1 to numZones) = mode 1, (numZones+1 to 2*numZones) = mode 2, (2*numZones+1 to 3*numZones) = mode 3, and so on for 9 modes.
	 * @return the mode represented in choice
	 */
	int getMode(int choice) const;

	/**
	 * Returns the destination of a particular choice
	 *
	 * @param choice an integer ranging from 1 to (numZones*numModes = 9828) representing a unique combination of mode and destination
	 * @return the destination represented in choice
	 */
	int getDestination(int choice) const;

	void setCbdOrgZone(int cbdOrg)
	{
		this->cbdOrgZone = cbdOrg;
	}

	int getOrigin() const
	{
		return origin;
	}

	void setOrigin(int origin)
	{
		this->origin = origin;
	}
};
} // end namespace sim_mob
