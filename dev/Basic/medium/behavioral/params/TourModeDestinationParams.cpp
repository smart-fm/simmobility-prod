//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TourModeDestinationParams.hpp"
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;
using namespace medium;

namespace
{
/**
 * function to set drive1 and motorcycle availability for tours/stops
 * @param stop whether this availability is being set for an intermediate stop (false implies it is being set for a tour)
 * @param personParams person parameters to infer license and vehicle ownership info
 * @param driveAvailable output param for car availability
 * @param motorAvailable output param for motorcycle availability
 */
void setCarAndMotorAvailability(bool stop, const PersonParams& personParams, bool& driveAvailable, bool& motorAvailable)
{
	switch(personParams.getVehicleOwnershipOption())
	{
	case VehicleOwnershipOption::NO_VEHICLE:
	case VehicleOwnershipOption::INVALID:
	{
		driveAvailable = false;
		motorAvailable = false;
		break;
	}
	case VehicleOwnershipOption::ONE_PLUS_MOTOR_ONLY:
	{
		driveAvailable = false;
		motorAvailable = personParams.getMotorLicense();
		break;
	}
	case VehicleOwnershipOption::ONE_OP_CAR_W_WO_MOTOR:
	{
		driveAvailable = stop * personParams.hasDrivingLicence();
		motorAvailable = personParams.getMotorLicense();
		break;
	}
	case VehicleOwnershipOption::ONE_NORMAL_CAR_ONLY:
	{
		driveAvailable = personParams.hasDrivingLicence();
		motorAvailable = false;
		break;
	}
	case VehicleOwnershipOption::ONE_NORMAL_CAR_AND_ONE_PLUS_MOTOR:
	case VehicleOwnershipOption::TWO_PLUS_NORMAL_CAR_W_WO_MOTOR:
	{
		driveAvailable = personParams.hasDrivingLicence();
		motorAvailable = personParams.getMotorLicense();
		break;
	}
	}
}
} // end anonymous namespace

TourModeDestinationParams::TourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		const PersonParams& personParams, StopType tourType,
		const std::vector<OD_Pair>& unavailableODs) :
		ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, tourType, personParams.getHomeLocation(), unavailableODs),
			drive1Available(false), motorAvailable(false), modeForParentWorkTour(0), costIncrease(0)
{
	setCarAndMotorAvailability(false, personParams, drive1Available, motorAvailable);
}

TourModeDestinationParams::~TourModeDestinationParams()
{
}

double TourModeDestinationParams::getCostPublicFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubCost();
}

double TourModeDestinationParams::getCostPublicSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubCost();
}

double TourModeDestinationParams::getCostCarERPFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getCarCostErp();
}

double TourModeDestinationParams::getCostCarERPSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getCarCostErp();
}

double TourModeDestinationParams::getCostCarOPFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (amCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST);
}

double TourModeDestinationParams::getCostCarOPSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (pmCostsMap.at(destination).at(origin)->getDistance() * OPERATIONAL_COST);
}

double TourModeDestinationParams::getCostCarParking(int zoneId) const
{
	return (8 * zoneMap.at(zoneId)->getParkingRate());
}

double TourModeDestinationParams::getWalkDistance1(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubWalkt();
}

double TourModeDestinationParams::getWalkDistance2(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubWalkt();
}

double TourModeDestinationParams::getTT_PublicIvtFirst(int zoneId)
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubIvt();
}

double TourModeDestinationParams::getTT_PublicIvtSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubIvt();
}

double TourModeDestinationParams::getTT_CarIvtFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getCarIvt();
}

double TourModeDestinationParams::getTT_CarIvtSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getCarIvt();
}

double TourModeDestinationParams::getTT_PublicOutFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubOut();
}

double TourModeDestinationParams::getTT_PublicOutSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubOut();
}

double TourModeDestinationParams::getAvgTransferNumber(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (amCostsMap.at(origin).at(destination)->getAvgTransfer() + pmCostsMap.at(destination).at(origin)->getAvgTransfer()) / 2;
}

int TourModeDestinationParams::getCentralDummy(int zone) const
{
	return zoneMap.at(zone)->getCentralDummy();
}

int TourModeDestinationParams::getCbdDummy(int zone) const
{
	return zoneMap.at(zone)->getCbdDummy();
}

StopType TourModeDestinationParams::getTourPurpose() const
{
	return purpose;
}

double TourModeDestinationParams::getShop(int zone) const
{
	return zoneMap.at(zone)->getShop();
}

double TourModeDestinationParams::getEmployment(int zone) const
{
	return zoneMap.at(zone)->getEmployment();
}

double TourModeDestinationParams::getPopulation(int zone) const
{
	return zoneMap.at(zone)->getPopulation();
}

double TourModeDestinationParams::getArea(int zone) const
{
	return zoneMap.at(zone)->getArea();
}

void TourModeDestinationParams::setDrive1Available(bool drive1Available)
{
	this->drive1Available = drive1Available;
}

double TourModeDestinationParams::getCostIncrease() const
{
	return costIncrease;
}

int TourModeDestinationParams::isAvailable_TMD(int choiceId) const
{
	/* 1. if the destination == origin, the destination is not available.
	 * 2. public bus, private bus and MRT/LRT are only available if AM[(origin,destination)][’pub_ivt’]>0 and PM[(destination,origin)][’pub_ivt’]>0
	 * 3. shared2, shared3+, taxi and motorcycle are available to all.
	 * 4. Walk is only avaiable if (AM[(origin,destination)][’distance’]<=2 and PM[(destination,origin)][’distance’]<=2)
	 * 5. drive alone is available when for the agent, has_driving_license * one_plus_car == True
	 */
	int numZones = zoneMap.size();
	int numModes = 9;
	if (choiceId < 1 || choiceId > numModes * numZones)
	{
		throw std::runtime_error("isAvailable()::invalid choice id for mode-destination model");
	}

	int zoneId = choiceId % numZones;
	if (zoneId == 0)
	{ // zoneId will become zero for the last zone
		zoneId = numZones;
	}
	int destination = zoneMap.at(zoneId)->getZoneCode();
	// the destination same as origin is not available
	if (origin == destination)
	{
		return 0;
	}

	// check if destination is unavailable
	if(isUnavailable(origin, destination))
	{
		return 0; // destination is unavailable due to lack of cost data
	}

	// bus 1-1092; mrt 1093 - 2184; private bus 2185 - 3276; same result for the three modes
	if (choiceId <= 3 * numZones)
	{
		return (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0 && amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
	}
	// drive1 3277 - 4368
	if (choiceId <= 4 * numZones)
	{
		return drive1Available;
	}
	// share2 4369 - 5460
	if (choiceId <= 5 * numZones)
	{
		// share2 is available to all
		return 1;
	}
	// share3 5461 - 6552
	if (choiceId <= 6 * numZones)
	{
		// share3 is available to all
		return 1;
	}
	// motor 6553 - 7644
	if (choiceId <= 7 * numZones)
	{
		return motorAvailable;
	}
	// walk 7645 - 8736
	if (choiceId <= 8 * numZones)
	{
		return (amCostsMap.at(origin).at(destination)->getDistance() <= MAX_WALKING_DISTANCE
				&& pmCostsMap.at(destination).at(origin)->getDistance() <= MAX_WALKING_DISTANCE);
	}
	// taxi 8737 - 9828
	if (choiceId <= 9 * numZones)
	{
		// taxi is available to all
		return 1;
	}
	return 0;
}

int TourModeDestinationParams::getModeForParentWorkTour() const
{
	return modeForParentWorkTour;
}

void TourModeDestinationParams::setModeForParentWorkTour(int modeForParentWorkTour)
{
	this->modeForParentWorkTour = modeForParentWorkTour;
}

int sim_mob::medium::TourModeDestinationParams::isCbdOrgZone() const
{
	return cbdOrgZone;
}

StopModeDestinationParams::StopModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		const PersonParams& personParams, const Stop* stop, int originCode,
		const std::vector<OD_Pair>& unavailableODs) :
		ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, stop->getStopType(), originCode, unavailableODs), homeZone(personParams.getHomeLocation()),
			driveAvailable(false), motorAvailable(false), tourMode(stop->getParentTour().getTourMode()),
			firstBound(stop->isInFirstHalfTour())
{
	setCarAndMotorAvailability(true, personParams, driveAvailable, motorAvailable);
}

StopModeDestinationParams::~StopModeDestinationParams()
{
}

double StopModeDestinationParams::getCostCarParking(int zone) const
{
	return 0;  // parking cost is always 0 for intermediate stops
}

double StopModeDestinationParams::getCostCarOP(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST)/2
				+(amCostsMap.at(destination).at(homeZone)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(destination).at(homeZone)->getDistance() * OPERATIONAL_COST )/2
				-(amCostsMap.at(origin).at(homeZone)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(origin).at(homeZone)->getDistance() * OPERATIONAL_COST)/2);
}

double StopModeDestinationParams::getCarCostERP(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getCarCostErp() + pmCostsMap.at(origin).at(destination)->getCarCostErp())/2
				+(amCostsMap.at(destination).at(homeZone)->getCarCostErp() + pmCostsMap.at(destination).at(homeZone)->getCarCostErp())/2
				-(amCostsMap.at(origin).at(homeZone)->getCarCostErp() + pmCostsMap.at(origin).at(homeZone)->getCarCostErp())/2);
}

double StopModeDestinationParams::getCostPublic(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubCost() + pmCostsMap.at(origin).at(destination)->getPubCost())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubCost() + pmCostsMap.at(destination).at(homeZone)->getPubCost())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubCost() + pmCostsMap.at(origin).at(homeZone)->getPubCost())/2);
}

double StopModeDestinationParams::getTT_CarIvt(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getCarIvt() + pmCostsMap.at(origin).at(destination)->getCarIvt())/2
				+(amCostsMap.at(destination).at(homeZone)->getCarIvt() + pmCostsMap.at(destination).at(homeZone)->getCarIvt())/2
				-(amCostsMap.at(origin).at(homeZone)->getCarIvt() + pmCostsMap.at(origin).at(homeZone)->getCarIvt())/2);
}

double StopModeDestinationParams::getTT_PubIvt(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubIvt() + pmCostsMap.at(origin).at(destination)->getPubIvt())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubIvt() + pmCostsMap.at(destination).at(homeZone)->getPubIvt())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubIvt() + pmCostsMap.at(origin).at(homeZone)->getPubIvt())/2);
}

double StopModeDestinationParams::getTT_PubOut(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubOut() + pmCostsMap.at(origin).at(destination)->getPubOut())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubOut() + pmCostsMap.at(destination).at(homeZone)->getPubOut())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubOut() + pmCostsMap.at(origin).at(homeZone)->getPubOut())/2);
}

double StopModeDestinationParams::getWalkDistanceFirst(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return (amCostsMap.at(origin).at(destination)->getDistance()
				+ amCostsMap.at(destination).at(homeZone)->getDistance()
				- amCostsMap.at(origin).at(homeZone)->getDistance());
}

double StopModeDestinationParams::getWalkDistanceSecond(int zone) const
{
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone)
	{ return 0; }
	return (pmCostsMap.at(origin).at(destination)->getDistance()
				+ pmCostsMap.at(destination).at(homeZone)->getDistance()
				- pmCostsMap.at(origin).at(homeZone)->getDistance());
}

int StopModeDestinationParams::getCentralDummy(int zone) const
{
	return zoneMap.at(zone)->getCentralDummy();
}

int StopModeDestinationParams::getCbdDummy(int zone) const
{
	return zoneMap.at(zone)->getCbdDummy();
}

StopType StopModeDestinationParams::getTourPurpose() const
{
	return purpose;
}

double StopModeDestinationParams::getShop(int zone) const
{
	return zoneMap.at(zone)->getShop();
}

double StopModeDestinationParams::getEmployment(int zone) const
{
	return zoneMap.at(zone)->getEmployment();
}

double StopModeDestinationParams::getPopulation(int zone) const
{
	return zoneMap.at(zone)->getPopulation();
}

double StopModeDestinationParams::getArea(int zone) const
{
	return zoneMap.at(zone)->getArea();
}

int StopModeDestinationParams::isAvailable_IMD(int choiceId) const
{
	/* 1. if the destination == origin, the destination is not available.
	 * 2. public bus, private bus and MRT/LRT are only available if AM[(origin,destination)][’pub_ivt’]>0 and PM[(destination,origin)][’pub_ivt’]>0
	 * 3. shared2, shared3+, taxi and motorcycle are available to all.
	 * 4. Walk is only avaiable if (AM[(origin,destination)][’distance’]<=2 and PM[(destination,origin)][’distance’]<=2)
	 * 5. drive alone is available when for the agent, has_driving_license * one_plus_car == True
	 */
	int numZones = zoneMap.size();
	int numModes = 9;
	if (choiceId < 1 || choiceId > numZones*numModes)
	{
		throw std::runtime_error("isAvailable()::invalid choice id for mode-destination model");
	}
	int zoneId = choiceId % numZones;
	if(zoneId == 0) { zoneId = numZones; } // zoneId will become zero for the last zone
	int destination = zoneMap.at(zoneId)->getZoneCode();

	if (origin == destination) { return 0; } // the destination same as origin is not available

	// check if destination is unavailable
	if(isUnavailable(origin, destination)
			|| isUnavailable(origin, homeZone)
			|| isUnavailable(destination, homeZone))
	{ return 0; } // destination is unavailable due to lack of cost data

	// bus 1-1092; mrt 1093 - 2184; private bus 2185 - 3276; same result for the three modes
	if (choiceId <= 3 * numZones)
	{
		bool avail = (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0
				&& amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
		switch(tourMode)
		{
		case 1: return avail;
		case 2: return avail;
		case 3: return avail;
		case 4: return 0;
		case 5: return 0;
		case 6: return 0;
		case 7: return 0;
		case 8: return 0;
		case 9: return 0;
		}
	}

	// drive1 3277 - 4368
	if (choiceId <= 4 * numZones)
	{
		switch(tourMode)
		{
		case 1: return driveAvailable;
		case 2: return driveAvailable;
		case 3: return driveAvailable;
		case 4: return driveAvailable;
		case 5: return driveAvailable;
		case 6: return driveAvailable;
		case 7: return 0;
		case 8: return 0;
		case 9: return 0;
		}
	}

	// share2 4369 - 5460
	if (choiceId <= 6 * numZones)
	{
		switch(tourMode)
		{
		case 1: return 1;
		case 2: return 1;
		case 3: return 1;
		case 4: return 0;
		case 5: return 1;
		case 6: return 1;
		case 7: return 0;
		case 8: return 0;
		case 9: return 0;
		}
	}

	// motor 6553 - 7644
	if (choiceId <= 7 * numZones)
	{
		switch(tourMode)
		{
		case 1: return motorAvailable;
		case 2: return motorAvailable;
		case 3: return motorAvailable;
		case 4: return 0;
		case 5: return motorAvailable;
		case 6: return motorAvailable;
		case 7: return motorAvailable;
		case 8: return 0;
		case 9: return 0;
		}
	}
	// walk 7645 - 8736
	if (choiceId <= 8 * numZones)
	{
		return (amCostsMap.at(origin).at(destination)->getDistance() <= MAX_WALKING_DISTANCE
				&& pmCostsMap.at(destination).at(origin)->getDistance() <= MAX_WALKING_DISTANCE);
	}
	// taxi 8737 - 9828
	if (choiceId <= 9 * numZones)
	{
		// taxi is available to all
		switch(tourMode)
		{
		case 1: return 1;
		case 2: return 1;
		case 3: return 1;
		case 4: return 0;
		case 5: return 1;
		case 6: return 1;
		case 7: return 0;
		case 8: return 0;
		case 9: return 1;
		}
	}
	return 0;
}

int StopModeDestinationParams::isFirstBound() const
{
	return firstBound;
}

int StopModeDestinationParams::isSecondBound() const
{
	return !firstBound;
}

int sim_mob::medium::StopModeDestinationParams::isCbdOrgZone() const
{
	return cbdOrgZone;
}

int sim_mob::medium::SubTourParams::getTimeWindowAvailability(size_t timeWnd) const
{
	return timeWindowAvailability[timeWnd - 1].getAvailability();
}

void sim_mob::medium::SubTourParams::initTimeWindows(double startTime, double endTime)
{
	if (!timeWindowAvailability.empty())
	{
		timeWindowAvailability.clear();
	}
	size_t index = 0;
	for (double start = 1; start <= 48; start++)
	{
		for (double end = start; end <= 48; end++)
		{
			if (start >= startTime && end <= endTime)
			{
				timeWindowAvailability.push_back(TimeWindowAvailability(start, end, true));
				availabilityBit[index] = 1;
			}
			else
			{
				timeWindowAvailability.push_back(TimeWindowAvailability(start, end, false));
			}
			index++;
		}
	}
}

void sim_mob::medium::SubTourParams::blockTime(double startTime, double endTime)
{
	if (startTime <= endTime)
	{
		size_t index = 0;
		for (std::vector<TimeWindowAvailability>::iterator i = timeWindowAvailability.begin(); i != timeWindowAvailability.end(); i++, index++)
		{
			TimeWindowAvailability& twa = (*i);
			double start = twa.getStartTime();
			double end = twa.getEndTime();
			if ((start >= startTime && start <= endTime) || (end >= startTime && end <= endTime))
			{
				twa.setAvailability(false);
				availabilityBit[index] = 0;
			}
		}
	}
	else
	{
		std::stringstream errStream;
		errStream << "invalid time window was passed for blocking" << "|start: " << startTime << "|end: " << endTime << std::endl;
		throw std::runtime_error(errStream.str());
	}
}

sim_mob::medium::SubTourParams::SubTourParams(const Tour& parentTour) :
		subTourPurpose(parentTour.getTourType()), usualLocation(parentTour.isUsualLocation()), tourMode(parentTour.getTourMode()),
			firstOfMultipleTours(parentTour.isFirstTour()), subsequentOfMultipleTours(!parentTour.isFirstTour())
{
	const Stop* primaryStop = parentTour.getPrimaryStop();
	initTimeWindows(primaryStop->getArrivalTime(), primaryStop->getDepartureTime());
}

sim_mob::medium::SubTourParams::~SubTourParams()
{
}

bool sim_mob::medium::SubTourParams::allWindowsUnavailable()
{
	return availabilityBit.none();
}
