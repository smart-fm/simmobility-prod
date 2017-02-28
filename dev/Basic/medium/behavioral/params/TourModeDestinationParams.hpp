//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <vector>
#include "behavioral/params/ModeDestinationParams.hpp"
#include "behavioral/params/PersonParams.hpp"
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/StopType.hpp"
#include "behavioral/PredayClasses.hpp"
#include "behavioral/PredayUtils.hpp"

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
    TourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, const PersonParams& personParams, StopType tourType, int numModes,
			const std::vector<OD_Pair>& unavailableODs);
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
    //void setDrive1Available(bool drive1Available);
	int isAvailable_TMD(int choiceId) const;
	int getModeForParentWorkTour() const;
	void setModeForParentWorkTour(int modeForParentWorkTour);
	int getCbdDummy(int zone) const;
	int isCbdOrgZone() const;
	double getCostIncrease() const;

private:
    /*bool drive1Available;
    bool motorAvailable;*/

    std::unordered_map<int, bool> modeAvailability;

	int modeForParentWorkTour;
	double costIncrease;
};

class StopModeDestinationParams: public ModeDestinationParams
{
public:
	StopModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, const PersonParams& personParams, const Stop* stop,
            int originCode, int numModes, const std::vector<OD_Pair>& unavailableODs);
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
    /*bool driveAvailable;
    bool motorAvailable;*/

    std::unordered_map<int, bool> modeAvailability;

	int tourMode;
	bool firstBound;
};

/**
 * Simple class to store information pertaining sub tour model
 * \note This class is used by the mid-term behavior models.
 *
 * \author Harish Loganathan
 */
class SubTourParams
{
public:
	SubTourParams(const Tour& tour);
	virtual ~SubTourParams();

	int isFirstOfMultipleTours() const
	{
		return firstOfMultipleTours;
	}

	void setFirstOfMultipleTours(bool firstOfMultipleTours)
	{
		this->firstOfMultipleTours = firstOfMultipleTours;
	}

	int isSubsequentOfMultipleTours() const
	{
		return subsequentOfMultipleTours;
	}

	void setSubsequentOfMultipleTours(bool subsequentOfMultipleTours)
	{
		this->subsequentOfMultipleTours = subsequentOfMultipleTours;
	}

	int getTourMode() const
	{
		return tourMode;
	}

	void setTourMode(int tourMode)
	{
		this->tourMode = tourMode;
	}

	int isUsualLocation() const
	{
		return usualLocation;
	}

	void setUsualLocation(bool usualLocation)
	{
		this->usualLocation = usualLocation;
	}

	int getSubTourPurpose() const
	{
		return subTourPurpose;
	}

	void setSubTourPurpose(StopType subTourpurpose)
	{
		this->subTourPurpose = subTourpurpose;
	}

	int isCbdDestZone() const
	{
		return (cbdDestZone ? 1 : 0);
	}

	void setCbdDestZone(int cbdDestZone)
	{
		this->cbdDestZone = cbdDestZone;
	}

	int isCbdOrgZone() const
	{
		return (cbdOrgZone ? 1 : 0);
	}

	void setCbdOrgZone(int cbdOrgZone)
	{
		this->cbdOrgZone = cbdOrgZone;
	}

	/**
	 * make time windows between startTime and endTime available
	 *
	 * @param startTime start time of available window
	 * @param endTime end time of available window
	 */
	void initTimeWindows(double startTime, double endTime);

	/**
	 * get the availability for a time window for tour
	 *
	 * @param timeWnd time window index to check availability
	 *
	 * @return 0 if time window is not available; 1 if available
	 *
	 * NOTE: This function is invoked from the Lua layer. The return type is int in order to be compatible with Lua.
	 *       Lua does not support boolean types.
	 */
	int getTimeWindowAvailability(size_t timeWnd) const;

	/**
	 * make time windows between startTime and endTime unavailable
	 *
	 * @param startTime start time
	 * @param endTime end time
	 */
	void blockTime(double startTime, double endTime);

	/**
	 * check if all time windows are unavailable
	 *
	 * @return true if all time windows are unavailable; false otherwise
	 */
	bool allWindowsUnavailable();

private:
	/**
	 * mode choice for parent tour
	 */
	int tourMode;

	/**
	 * parent tour is the first of many tours for person
	 */
	bool firstOfMultipleTours;

	/**
	 * parent tour is the 2+ of many tours for person
	 */
	bool subsequentOfMultipleTours;

	/**
	 * parent tour is to a usual location
	 */
	bool usualLocation;

	/**
	 * sub tour type
	 */
	StopType subTourPurpose;

	/**
	 * Time windows available for sub-tour.
	 */
	std::vector<sim_mob::TimeWindowAvailability> timeWindowAvailability;

	/**
	 * bitset of availablilities for fast checking
	 */
	std::bitset<1176> availabilityBit;

	int cbdOrgZone;
	int cbdDestZone;
};
} // end namespace medium
} // end namespace sim_mob
