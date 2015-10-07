//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/params/ModeDestinationParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/StopType.hpp"
#include "behavioral/PredayClasses.hpp"
#include "PersonParams.hpp"

namespace sim_mob
{
namespace medium
{

/**
 * Class to hold parameters for tour mode destination models
 * NOTE: this class is mapped to tmd models in corresponding lua scripts
 *
 * \author Harish Loganathan
 */
class TourModeDestinationParams: public ModeDestinationParams
{
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
	StopType getTourPurpose() const;
	double getShop(int zone) const;
	double getEmployment(int zone) const;
	double getPopulation(int zone) const;
	double getArea(int zone) const;
	void setDrive1Available(bool drive1Available);
	int isAvailable_TMD(int choiceId) const;
	int getModeForParentWorkTour() const;
	void setModeForParentWorkTour(int modeForParentWorkTour);
	int getCbdDummy(int zone) const;
	int isCbdOrgZone() const;
	double getCostIncrease() const;

private:
	bool drive1Available;
	int modeForParentWorkTour;
	double costIncrease;
};

class StopModeDestinationParams: public ModeDestinationParams
{
public:
	StopModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, const PersonParams& personParams, const Stop* stop,
			int originCode, const std::vector<OD_Pair>& unavailableODs);
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
	StopType getTourPurpose() const;
	double getShop(int zone) const;
	double getEmployment(int zone) const;
	double getPopulation(int zone) const;
	double getArea(int zone) const;
	int isAvailable_IMD(int choiceId) const;
	int isFirstBound() const;
	int isSecondBound() const;
	int isCbdOrgZone() const;
	int getCbdDummy(int zone) const;

private:
	int homeZone;
	int driveAvailable;
	int tourMode;
	bool firstBound;
	const std::vector<OD_Pair>& unavailableODs;

};

} // end namespace medium
} // end namespace sim_mob
