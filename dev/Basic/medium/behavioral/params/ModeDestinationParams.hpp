//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TourModeDestinationParams.hpp
 *
 *  Created on: Nov 30, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include <cmath>
#include <string>
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/PredayClasses.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "database/ZoneCostMongoDao.hpp"
#include "logging/Log.hpp"
#include "mongo/client/dbclient.h"
#include "PersonParams.hpp"

namespace sim_mob {
namespace medium {

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
};

class TourModeDestinationParams : public ModeDestinationParams {
public:
	TourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, const PersonParams& personParams, StopType tourType);
	virtual ~TourModeDestinationParams();

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
	int getCbdDummy(int zone) const;
	StopType getTourPurpose() const;
	double getShop(int zone) const;
	double getEmployment(int zone) const;
	double getPopulation(int zone) const;
	double getArea(int zone) const;
	void setDrive1Available(bool drive1Available);
	int isAvailable_TMD(int choiceId) const;
	int getModeForParentWorkTour() const;
	void setModeForParentWorkTour(int modeForParentWorkTour);

private:
	bool drive1Available;
	/**mode for parent work tour in case of sub tours*/
	int modeForParentWorkTour;
};

class StopModeDestinationParams : public ModeDestinationParams {
public:
	StopModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
			const PersonParams& personParams, const Stop* stop, int originCode, const std::vector<OD_Pair>& unavailableODs);
	virtual ~StopModeDestinationParams();
	double getCostCarParking(int zone) const;
	double getCostCarOP(int zone) const;
	double getCarCostERP(int zone) const;
	double getCostPublic(int zone) const;
	double getTT_CarIvt(int zone) const;
	double getTT_PubIvt(int zone) const;
	double getTT_PubOut(int zone) const;
	double getWalkDistanceFirst(int zone) const;
	double getWalkDistanceSecond(int zone) const;
	int getCentralDummy(int zone) const;
	int getCbdDummy(int zone) const;
	StopType getTourPurpose() const ;
	double getShop(int zone) const;
	double getEmployment(int zone) const;
	double getPopulation(int zone) const;
	double getArea(int zone) const;
	int isAvailable_IMD(int choiceId) const;
	int isFirstBound() const;
	int isSecondBound() const;

private:
	int homeZone;
	int driveAvailable;
	int tourMode;
	bool firstBound;
	const std::vector<OD_Pair>& unavailableODs;

};

} // end namespace medium
} // end namespace sim_mob
