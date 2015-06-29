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
#include "behavioral/params/ModeDestinationParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/StopType.hpp"
#include "PredayPersonParams.hpp"

namespace sim_mob
{
class LogsumTourModeDestinationParams : public ModeDestinationParams {
public:
	LogsumTourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, const PredayPersonParams& personParams, StopType tourType);
	virtual ~LogsumTourModeDestinationParams();

	double getCostPublicFirst(int zoneId) const;
	double getCostPublicSecond(int zoneId) const;
	double getCostCarERPFirst(int zoneId) const;
	double getCostCarERPSecond(int zoneId) const;
	double getCostCarOPFirst(int zoneId) const;
	double getCostCarOPSecond(int zoneId) const;
	double getCostCarParking(int zoneId) const;
	double getWalkDistance1(int zoneId) const;
	double getWalkDistance2(int zoneId) const;
	double getTT_PublicIvtFirst(int zoneId);
	double getTT_PublicIvtSecond(int zoneId) const;
	double getTT_CarIvtFirst(int zoneId) const;
	double getTT_CarIvtSecond(int zoneId) const;
	double getTT_PublicOutFirst(int zoneId) const;
	double getTT_PublicOutSecond(int zoneId) const;
	double getAvgTransferNumber(int zoneId) const;
	int getCentralDummy(int zone) const;
	StopType getTourPurpose() const;
	double getShop(int zone) const;
	double getEmployment(int zone) const;
	double getPopulation(int zone) const;
	double getArea(int zone) const;
	void setDrive1Available(bool drive1Available);
	int isAvailable_TMD(int choiceId) const;
	int getCbdDummy(int zone) const;
	int isCbdOrgZone() const;

private:
	bool drive1Available;
	/**mode for parent work tour in case of sub tours*/
	int modeForParentWorkTour;
};
}
