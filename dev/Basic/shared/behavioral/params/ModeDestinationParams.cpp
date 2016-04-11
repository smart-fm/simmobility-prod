//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ModeDestinationParams.hpp"
#include "LogsumTourModeDestinationParams.hpp"

#include <algorithm>

using namespace std;
using namespace sim_mob;

namespace
{
const double WALKABLE_DISTANCE = 3.0; //km
const double OPERATIONAL_COST = 0.147;

const std::vector<OD_Pair> unavailableODsDummy;
}

ModeDestinationParams::ModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		StopType purpose, int originCode, const std::vector<OD_Pair>& unavailableODs) :
		zoneMap(zoneMap), amCostsMap(amCostsMap), pmCostsMap(pmCostsMap), purpose(purpose), origin(originCode), OPERATIONAL_COST(0.147),
			MAX_WALKING_DISTANCE(3), cbdOrgZone(false), unavailableODs(unavailableODs)
{
}

ModeDestinationParams::~ModeDestinationParams()
{
}

int ModeDestinationParams::getMode(int choice) const
{
	int nZones = zoneMap.size();
	int nModes = 9;
	if (choice < 1 || choice > nZones * nModes)
	{
		throw std::runtime_error("getMode()::invalid choice id for mode-destination model");
	}
	return ((choice - 1) / nZones + 1);
}

int ModeDestinationParams::getDestination(int choice) const
{
	int nZones = zoneMap.size();
	int nModes = 9;
	if (choice < 1 || choice > nZones * nModes)
	{
		throw std::runtime_error("getDestination()::invalid choice id for mode-destination model");
	}
	int zoneId = choice % nZones;
	if (zoneId == 0)
	{ // zoneId will become zero for zone 1092.
		zoneId = nZones;
	}
	return zoneId;
}

bool sim_mob::ModeDestinationParams::isUnavailable(int origin, int destination) const
{
	//int origin08 = MTZ12_MTZ08_Map.at(origin);
	//int destin08 = MTZ12_MTZ08_Map.at(destination);
	OD_Pair orgDest = OD_Pair(origin, destination);
	return binary_search(unavailableODs.begin(), unavailableODs.end(), orgDest);
}

LogsumTourModeDestinationParams::LogsumTourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		const PredayPersonParams& personParams, StopType tourType) :
		ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, tourType, personParams.getHomeLocation(), unavailableODsDummy),
			drive1Available(personParams.hasDrivingLicence() * personParams.getCarOwn()),
			motorAvailable(personParams.getMotorLicense() * personParams.getMotorOwn()),
			modeForParentWorkTour(0), costIncrease(1)
{
}

LogsumTourModeDestinationParams::~LogsumTourModeDestinationParams()
{
}

double LogsumTourModeDestinationParams::getCostPublicFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubCost();
}

double LogsumTourModeDestinationParams::getCostPublicSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubCost();
}

double LogsumTourModeDestinationParams::getCostCarERPFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getCarCostErp();
}

double LogsumTourModeDestinationParams::getCostCarERPSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getCarCostErp();
}

double LogsumTourModeDestinationParams::getCostCarOPFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (amCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST);
}

double LogsumTourModeDestinationParams::getCostCarOPSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (pmCostsMap.at(destination).at(origin)->getDistance() * OPERATIONAL_COST);
}

double LogsumTourModeDestinationParams::getCostCarParking(int zoneId) const
{
	return (8 * zoneMap.at(zoneId)->getParkingRate());
}

double LogsumTourModeDestinationParams::getWalkDistance1(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubWalkt();
}

double LogsumTourModeDestinationParams::getWalkDistance2(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubWalkt();
}

double LogsumTourModeDestinationParams::getTT_PublicIvtFirst(int zoneId)
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubIvt();
}

double LogsumTourModeDestinationParams::getTT_PublicIvtSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubIvt();
}

double LogsumTourModeDestinationParams::getTT_CarIvtFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getCarIvt();
}

double LogsumTourModeDestinationParams::getTT_CarIvtSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getCarIvt();
}

double LogsumTourModeDestinationParams::getTT_PublicOutFirst(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return amCostsMap.at(origin).at(destination)->getPubOut();
}

double LogsumTourModeDestinationParams::getTT_PublicOutSecond(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return pmCostsMap.at(destination).at(origin)->getPubOut();
}

double LogsumTourModeDestinationParams::getAvgTransferNumber(int zoneId) const
{
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination)
	{
		return 0;
	}
	return (amCostsMap.at(origin).at(destination)->getAvgTransfer() + pmCostsMap.at(destination).at(origin)->getAvgTransfer()) / 2;
}

int LogsumTourModeDestinationParams::getCentralDummy(int zone) const
{
	return zoneMap.at(zone)->getCentralDummy();
}

StopType LogsumTourModeDestinationParams::getTourPurpose() const
{
	return purpose;
}

double LogsumTourModeDestinationParams::getShop(int zone) const
{
	return zoneMap.at(zone)->getShop();
}

double LogsumTourModeDestinationParams::getEmployment(int zone) const
{
	return zoneMap.at(zone)->getEmployment();
}

double LogsumTourModeDestinationParams::getPopulation(int zone) const
{
	return zoneMap.at(zone)->getPopulation();
}

double LogsumTourModeDestinationParams::getArea(int zone) const
{
	return zoneMap.at(zone)->getArea();
}

void LogsumTourModeDestinationParams::setDrive1Available(bool drive1Available)
{
	this->drive1Available = drive1Available;
}

double LogsumTourModeDestinationParams::getCostIncrease() const
{
	return costIncrease;
}

int LogsumTourModeDestinationParams::isAvailable_TMD(int choiceId) const
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
		// share3 is available to all
		return 1;
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

int sim_mob::LogsumTourModeDestinationParams::getCbdDummy(int zone) const
{
	return zoneMap.at(zone)->getCbdDummy();
}

int sim_mob::LogsumTourModeDestinationParams::isCbdOrgZone() const
{
	return cbdOrgZone;
}

sim_mob::LogsumTourModeParams::LogsumTourModeParams(const ZoneParams* znOrgObj, const ZoneParams* znDesObj, const CostParams* amObj, const CostParams* pmObj,
		const PredayPersonParams& personParams, StopType tourType) :
		stopType(tourType), costIncrease(1), costCarParking(znDesObj->getParkingRate()), centralZone(znDesObj->getCentralDummy()),
			cbdOrgZone(znOrgObj->getCbdDummy()), cbdDestZone(znDesObj->getCbdDummy()), residentSize(znOrgObj->getResidentWorkers()),
			workOP(znDesObj->getEmployment()), educationOP(znDesObj->getTotalEnrollment()), originArea(znOrgObj->getArea()),
			destinationArea(znDesObj->getArea())
{
	if (amObj && pmObj)
	{
		costPublicFirst = amObj->getPubCost();
		costPublicSecond = pmObj->getPubCost();
		costCarERP_First = amObj->getCarCostErp();
		costCarERP_Second = pmObj->getCarCostErp();
		costCarOP_First = amObj->getDistance() * OPERATIONAL_COST;
		costCarOP_Second = pmObj->getDistance() * OPERATIONAL_COST;
		walkDistance1 = amObj->getDistance();
		walkDistance2 = pmObj->getDistance();
		ttPublicIvtFirst = amObj->getPubIvt();
		ttPublicIvtSecond = pmObj->getPubIvt();
		ttPublicWaitingFirst = amObj->getPubWtt();
		ttPublicWaitingSecond = pmObj->getPubWtt();
		ttPublicWalkFirst = amObj->getPubWalkt();
		ttPublicWalkSecond = pmObj->getPubWalkt();
		ttCarIvtFirst = amObj->getCarIvt();
		ttCarIvtSecond = pmObj->getCarIvt();
		avgTransfer = (amObj->getAvgTransfer() + pmObj->getAvgTransfer()) / 2;

		//set availabilities
		drive1Available = personParams.hasDrivingLicence() * personParams.getCarOwnNormal();
		share2Available = 1;
		share3Available = 1;
		publicBusAvailable = (amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		mrtAvailable = (amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		privateBusAvailable = (amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		walkAvailable = (amObj->getPubIvt() <= WALKABLE_DISTANCE && pmObj->getPubIvt() <= WALKABLE_DISTANCE);
		taxiAvailable = 1;
		motorAvailable = 1;
	}
	else
	{
		costPublicFirst = 0;
		costPublicSecond = 0;
		costCarERP_First = 0;
		costCarERP_Second = 0;
		costCarOP_First = 0;
		costCarOP_Second = 0;
		walkDistance1 = 0;
		walkDistance2 = 0;
		ttPublicIvtFirst = 0;
		ttPublicIvtSecond = 0;
		ttPublicWaitingFirst = 0;
		ttPublicWaitingSecond = 0;
		ttPublicWalkFirst = 0;
		ttPublicWalkSecond = 0;
		ttCarIvtFirst = 0;
		ttCarIvtSecond = 0;
		avgTransfer = 0;

		//set availabilities
		drive1Available = personParams.hasDrivingLicence() * personParams.getCarOwnNormal();
		share2Available = 1;
		share3Available = 1;
		publicBusAvailable = 1;
		mrtAvailable = 1;
		privateBusAvailable = 1;
		walkAvailable = 1;
		taxiAvailable = 1;
		motorAvailable = 1;
	}
}

sim_mob::LogsumTourModeParams::~LogsumTourModeParams()
{
}
