//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <database/predaydao/ZoneCostSqlDao.hpp>
#include "TourModeDestinationParams.hpp"
#include "logging/Log.hpp"
#include "conf/ConfigManager.hpp" //jo

using namespace std;
using namespace sim_mob;
using namespace medium;
using namespace sim_mob::db;

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

void setModeAvailability(bool stop, const PersonParams& personParams, int numModes, std::unordered_map<int, bool>& modeAvailability, const ConfigParams& cfg)
{
    for (int mode = 1; mode <= numModes; ++mode)
    {
        modeAvailability[mode] = true;
    }

    switch(personParams.getVehicleOwnershipOption())
    {
    case VehicleOwnershipOption::NO_VEHICLE:
    case VehicleOwnershipOption::INVALID:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE || modeType == PVT_BIKE_MODE)
            {
                modeAvailability[mode] = false;
            }
        }
        break;
    }
    case VehicleOwnershipOption::ONE_PLUS_MOTOR_ONLY:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                modeAvailability[mode] = false;
            }
            if (modeType == PVT_BIKE_MODE)
            {
                modeAvailability[mode] = personParams.getMotorLicense();
            }
        }
        break;
    }
    case VehicleOwnershipOption::ONE_OP_CAR_W_WO_MOTOR:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                modeAvailability[mode] = stop * personParams.hasDrivingLicence();
            }
            if (modeType == PVT_BIKE_MODE)
            {
                modeAvailability[mode] = personParams.getMotorLicense();
            }
        }
        break;
    }
    case VehicleOwnershipOption::ONE_NORMAL_CAR_ONLY:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                modeAvailability[mode] = personParams.hasDrivingLicence();
            }
            if (modeType == PVT_BIKE_MODE)
            {
                modeAvailability[mode] = false;
            }
        }
        break;
    }
    case VehicleOwnershipOption::ONE_NORMAL_CAR_AND_ONE_PLUS_MOTOR:
    case VehicleOwnershipOption::TWO_PLUS_NORMAL_CAR_W_WO_MOTOR:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            int modeType = cfg.getTravelModeConfig(mode).type;
            if (modeType == PVT_CAR_MODE)
            {
                modeAvailability[mode] = personParams.hasDrivingLicence();
            }
            if (modeType == PVT_BIKE_MODE)
            {
                modeAvailability[mode] = personParams.getMotorLicense();
            }
        }
        break;
    }
    }
}

} // end anonymous namespace

TourModeDestinationParams::TourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
	const PersonParams& personParams, StopType tourType, const VehicleParams::VehicleDriveTrain& powerTrain, int numModes, const std::vector<OD_Pair>& unavailableODs) :
		ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, tourType, personParams.getHomeLocation(), powerTrain, numModes, unavailableODs),
		modeForParentWorkTour(0), costIncrease(0)
{
    //setCarAndMotorAvailability(false, personParams, drive1Available, motorAvailable);
    setModeAvailability(false, personParams, numModes, modeAvailability, ConfigManager::GetInstance().FullConfig());
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
	else
	{
		const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
		if (powertrain == VehicleParams::BEV or powertrain == VehicleParams::FCV)
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostBEV());
		}
		else if (powertrain == VehicleParams::HEV or powertrain == VehicleParams::PHEV)
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostHEV());
		}
		else // resorting to ICE
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostICE());
		}
	}
}

double TourModeDestinationParams::getCostCarOPSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	else
	{
		const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
		if (powertrain == VehicleParams::BEV or powertrain == VehicleParams::FCV)
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostBEV());
		}
		else if (powertrain == VehicleParams::HEV or powertrain == VehicleParams::PHEV)
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostHEV());
		}
		else // resorting to ICE
		{
			return (amCostsMap.at(origin).at(destination)->getDistance() * cfg.operationalCostICE());
		}
	}
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

/*void TourModeDestinationParams::setDrive1Available(bool drive1Available)
{
	this->drive1Available = drive1Available;
}*/

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
	//if destination is with zone without node
	bool result;
	//call to set the availability of zones without nodes to zero
	result=ZoneSqlDao::getZoneWithoutNode(destination);
	if(result)
	{
		return 0;
	}
	// check if destination is unavailable
	if(isUnavailable(origin, destination))
	{
		return 0; // destination is unavailable due to lack of cost data
	}

    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

    int modeType = cfg.getTravelModeConfig(getMode(choiceId)).type;

    switch (modeType) {
    case PT_TRAVEL_MODE:
    case PRIVATE_BUS_MODE:
    {
        return (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0 && amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
        break;
    }
    case PVT_CAR_MODE:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            if (cfg.getTravelModeConfig(mode).type == PVT_CAR_MODE)
            {
                return modeAvailability.at(mode);
            }
        }
        break;
    }
    case SHARING_MODE:
    case TAXI_MODE:
    {
        return 1;
        break;
    }
    case PVT_BIKE_MODE:
    {
        for (int mode = 1; mode <= numModes; ++mode)
        {
            if (cfg.getTravelModeConfig(mode).type == PVT_BIKE_MODE)
            {
                return modeAvailability.at(mode);
            }
        }
        break;
    }
    case WALK_MODE:
    {
        return (amCostsMap.at(origin).at(destination)->getDistance() <= MAX_WALKING_DISTANCE
                && pmCostsMap.at(destination).at(origin)->getDistance() <= MAX_WALKING_DISTANCE);
        break;
    }
    }

    return 0;

}

bool TourModeDestinationParams::areAllTourModeDestinationsUnavailable()
{
    int s = 0 ;
	int numberOfChoices = modeAvailability.size() * zoneMap.size();
    for (int i = 1 ; i<= numberOfChoices; i++ )
        s += (int)isAvailable_TMD(i) ;
    return s==0 ;
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
	const PersonParams& personParams, const Stop* stop, int originCode, const VehicleParams::VehicleDriveTrain& powerTrain, int numModes, const std::vector<OD_Pair>& unavailableODs) :
		ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, stop->getStopType(), originCode, powerTrain, numModes, unavailableODs), 
		homeZone(personParams.getHomeLocation()), tourMode(stop->getParentTour().getTourMode()),
		//firstBound(stop->isInFirstHalfTour()) //jo May12 reg restriction
 		firstBound(stop->isInFirstHalfTour()), personParams(personParams), parentTour(stop->getParentTour()) 
{
    //setCarAndMotorAvailability(true, personParams, driveAvailable, motorAvailable);
    setModeAvailability(true, personParams, numModes, modeAvailability, ConfigManager::GetInstance().FullConfig());
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
	if(origin == destination or destination == homeZone or origin == homeZone)
	{
		return 0;
	}
	else
	{
		const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();
		double operational_cost = cfg.operationalCostICE();
		if (powertrain == VehicleParams::BEV or powertrain == VehicleParams::FCV)
		{
			operational_cost = cfg.operationalCostBEV();
		}
		else if (powertrain == VehicleParams::HEV or powertrain == VehicleParams::PHEV)
		{
			operational_cost = cfg.operationalCostHEV();
		}
		else // resorting to ICE
		{
			operational_cost = cfg.operationalCostICE();
		}
		return ((amCostsMap.at(origin).at(destination)->getDistance() * operational_cost 
			+ pmCostsMap.at(origin).at(destination)->getDistance() * operational_cost)/2
			+(amCostsMap.at(destination).at(homeZone)->getDistance() * operational_cost 
			+ pmCostsMap.at(destination).at(homeZone)->getDistance() * operational_cost)/2
			-(amCostsMap.at(origin).at(homeZone)->getDistance() * operational_cost 
			+ pmCostsMap.at(origin).at(homeZone)->getDistance() * operational_cost)/2);
	}
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

int StopModeDestinationParams::getTourPurpose() const
{
	return int(purpose);
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

    const ConfigParams& cfg = ConfigManager::GetInstance().FullConfig();

	int numZones = zoneMap.size();
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

    int choiceModeType = cfg.getTravelModeConfig(getMode(choiceId)).type;
    int tourModeType = cfg.getTravelModeConfig(tourMode).type;

    switch(choiceModeType)
    {
    case PT_TRAVEL_MODE:
    case PRIVATE_BUS_MODE:
    {
        bool avail = (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0
                && amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
        switch(tourModeType)
        {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
        {
            return avail;
            break;
        }
        }
        break;
    }
    case PVT_CAR_MODE:
    {
        switch(tourModeType)
        {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
        case PVT_CAR_MODE:
        case SHARING_MODE:
        {
            for (int mode = 1; mode <= numModes; ++mode)
            {
                if (cfg.getTravelModeConfig(mode).type == PVT_CAR_MODE)
                {
                    return modeAvailability.at(mode);
                }
            }
            break;
        }
        }
        break;
    }
    case SHARING_MODE:
    {
        switch (tourModeType) {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
        case SHARING_MODE:
        {
            return 1;
            break;
        }
        }
        break;
    }
    case PVT_BIKE_MODE:
    {
        switch (tourModeType) {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
        case SHARING_MODE:
        case PVT_BIKE_MODE:
        {
            for (int mode = 1; mode <= numModes; ++mode)
            {
                if (cfg.getTravelModeConfig(mode).type == PVT_BIKE_MODE)
                {
                    return modeAvailability.at(mode);
                }
            }
            break;
        }
        }
        break;
    }
    case WALK_MODE:
    {
        return (amCostsMap.at(origin).at(destination)->getDistance() <= MAX_WALKING_DISTANCE
                && pmCostsMap.at(destination).at(origin)->getDistance() <= MAX_WALKING_DISTANCE);
        break;
    }
    case TAXI_MODE:
    {
        switch (tourModeType) {
        case PT_TRAVEL_MODE:
        case PRIVATE_BUS_MODE:
        case SHARING_MODE:
        case TAXI_MODE:
        {
            return 1;
            break;
        }
        }
        break;
    }
    }

    return 0;
}

bool StopModeDestinationParams::areAllStopModeDestinationsUnavailable()
{
	int s = 0 ;
	int numberOfChoices = modeAvailability.size() * zoneMap.size();
	for (int i = 1 ; i<= numberOfChoices; i++ )
		s += (int)isAvailable_IMD(i) ;
	return s==0 ;
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
	return timeWindowsLookup[timeWnd - 1].getAvailability();
}

void sim_mob::medium::SubTourParams::initTimeWindows(double startTime, double endTime)
{
	timeWindowsLookup.setAllUnavailable();
	timeWindowsLookup.setAvailable(startTime, endTime);
}

void sim_mob::medium::SubTourParams::blockTime(double startTime, double endTime)
{
	timeWindowsLookup.setUnavailable(startTime, endTime);
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
	return timeWindowsLookup.areAllUnavailable();
}
