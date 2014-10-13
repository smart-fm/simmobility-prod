//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ModeDestinationParams.hpp"

using namespace std;
using namespace sim_mob;
using namespace medium;

ModeDestinationParams::ModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap, StopType purpose, int originCode)
: zoneMap(zoneMap), amCostsMap(amCostsMap), pmCostsMap(pmCostsMap), purpose(purpose), origin(originCode), OPERATIONAL_COST(0.147), MAX_WALKING_DISTANCE(2)
{}

ModeDestinationParams::~ModeDestinationParams() {}

int ModeDestinationParams::getMode(int choice) const {
	int nZones = zoneMap.size();
	int nModes = 9;
	if (choice < 1 || choice > nZones*nModes) {
		throw std::runtime_error("getMode()::invalid choice id for mode-destination model");
	}
	return ((choice-1)/nZones + 1);
}

int ModeDestinationParams::getDestination(int choice) const {
	int nZones = zoneMap.size();
	int nModes = 9;
	if (choice < 1 || choice > nZones*nModes) {
		throw std::runtime_error("getDestination()::invalid choice id for mode-destination model");
	}
	int zoneId = choice % nZones;
	if(zoneId == 0) { // zoneId will become zero for zone 1092.
		zoneId = nZones;
	}
	return zoneId;
}

TourModeDestinationParams::TourModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		const PersonParams& personParams, StopType tourType)
: ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, tourType, personParams.getHomeLocation()),
  drive1Available(personParams.hasDrivingLicence() * personParams.getCarOwn()),
  modeForParentWorkTour(0),cbdOrgZone(false)
{}

TourModeDestinationParams::~TourModeDestinationParams() {}

double TourModeDestinationParams::getCostPublicFirst(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getPubCost();
}

double TourModeDestinationParams::getCostPublicSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getPubCost();
}

double TourModeDestinationParams::getCostCarERPFirst(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getCarCostErp();
}

double TourModeDestinationParams::getCostCarERPSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getCarCostErp();
}

double TourModeDestinationParams::getCostCarOPFirst(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return (amCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST);
}

double TourModeDestinationParams::getCostCarOPSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return (pmCostsMap.at(destination).at(origin)->getDistance() * OPERATIONAL_COST);
}

double TourModeDestinationParams::getCostCarParking(int zoneId) const {
	return (8*zoneMap.at(zoneId)->getParkingRate());
}

double TourModeDestinationParams::getWalkDistance1(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getPubWalkt();
}

double TourModeDestinationParams::getWalkDistance2(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getPubWalkt();
}

double TourModeDestinationParams::getTT_PublicIvtFirst(int zoneId) {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getPubIvt();
}

double TourModeDestinationParams::getTT_PublicIvtSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getPubIvt();
}

double TourModeDestinationParams::getTT_CarIvtFirst(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getCarIvt();
}

double TourModeDestinationParams::getTT_CarIvtSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getCarIvt();
}

double TourModeDestinationParams::getTT_PublicOutFirst(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return amCostsMap.at(origin).at(destination)->getPubOut();
}

double TourModeDestinationParams::getTT_PublicOutSecond(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return pmCostsMap.at(destination).at(origin)->getPubOut();
}

double TourModeDestinationParams::getAvgTransferNumber(int zoneId) const {
	int destination = zoneMap.at(zoneId)->getZoneCode();
	if (origin == destination) { return 0; }
	return (amCostsMap.at(origin).at(destination)->getAvgTransfer() + pmCostsMap.at(destination).at(origin)->getAvgTransfer())/2;
}

int TourModeDestinationParams::getCentralDummy(int zone) const {
	return zoneMap.at(zone)->getCentralDummy();
}

int TourModeDestinationParams::getCbdDummy(int zone) const {
	return zoneMap.at(zone)->getCbdDummy();
}

StopType TourModeDestinationParams::getTourPurpose() const {
	return purpose;
}

double TourModeDestinationParams::getShop(int zone) const {
	return zoneMap.at(zone)->getShop();
}

double TourModeDestinationParams::getEmployment(int zone) const {
	return zoneMap.at(zone)->getEmployment();
}

double TourModeDestinationParams::getPopulation(int zone) const {
	return zoneMap.at(zone)->getPopulation();
}

double TourModeDestinationParams::getArea(int zone) const {
	return zoneMap.at(zone)->getArea();
}

void TourModeDestinationParams::setDrive1Available(bool drive1Available) {
	this->drive1Available = drive1Available;
}

int TourModeDestinationParams::isAvailable_TMD(int choiceId) const {
	/* 1. if the destination == origin, the destination is not available.
	 * 2. public bus, private bus and MRT/LRT are only available if AM[(origin,destination)][’pub_ivt’]>0 and PM[(destination,origin)][’pub_ivt’]>0
	 * 3. shared2, shared3+, taxi and motorcycle are available to all.
	 * 4. Walk is only avaiable if (AM[(origin,destination)][’distance’]<=2 and PM[(destination,origin)][’distance’]<=2)
	 * 5. drive alone is available when for the agent, has_driving_license * one_plus_car == True
	 */
	int numZones = zoneMap.size();
	int numModes = 9;
	if (choiceId < 1 || choiceId > numModes*numZones) {
		throw std::runtime_error("isAvailable()::invalid choice id for mode-destination model");
	}

	int zoneId = choiceId % numZones;
	if(zoneId == 0) { // zoneId will become zero for the last zone
		zoneId = numZones;
	}
	int destination = zoneMap.at(zoneId)->getZoneCode();
	// the destination same as origin is not available
	if (origin == destination) {
		return 0;
	}
	// bus 1-1092; mrt 1093 - 2184; private bus 2185 - 3276; same result for the three modes
	if (choiceId <= 3 * numZones) {
		return (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0
				&& amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
	}
	// drive1 3277 - 4368
	if (choiceId <= 4 * numZones) {
		return drive1Available;
	}
	// share2 4369 - 5460
	if (choiceId <= 5 * numZones) {
		// share2 is available to all
		return 1;
	}
	// share3 5461 - 6552
	if (choiceId <= 6 * numZones) {
		// share3 is available to all
		return 1;
	}
	// motor 6553 - 7644
	if (choiceId <= 7 * numZones) {
		// share3 is available to all
		return 1;
	}
	// walk 7645 - 8736
	if (choiceId <= 8 * numZones) {
		return (amCostsMap.at(origin).at(destination)->getDistance() <= 2
				&& pmCostsMap.at(destination).at(origin)->getDistance() <= 2);
	}
	// taxi 8737 - 9828
	if (choiceId <= 9 * numZones) {
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

void sim_mob::medium::TourModeDestinationParams::setCbdOrgZone(bool cbdOrg)
{
	this->cbdOrgZone = cbdOrg;
}

int sim_mob::medium::TourModeDestinationParams::isCbdOrgZone() const
{
	return cbdOrgZone;
}

StopModeDestinationParams::StopModeDestinationParams(const ZoneMap& zoneMap, const CostMap& amCostsMap, const CostMap& pmCostsMap,
		const PersonParams& personParams, const Stop* stop, int originCode, const std::vector<OD_Pair>& unavailableODs)
: ModeDestinationParams(zoneMap, amCostsMap, pmCostsMap, stop->getStopType(), originCode),
  homeZone(personParams.getHomeLocation()),
  driveAvailable(personParams.hasDrivingLicence() * personParams.getCarOwn()),
  tourMode(stop->getParentTour().getTourMode()),
  firstBound(stop->isInFirstHalfTour()),
  unavailableODs(unavailableODs)
{}

StopModeDestinationParams::~StopModeDestinationParams() {}

double StopModeDestinationParams::getCostCarParking(int zone) const { return 0; } // parking cost is always 0 for intermediate stops

double StopModeDestinationParams::getCostCarOP(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(origin).at(destination)->getDistance() * OPERATIONAL_COST)/2
				+(amCostsMap.at(destination).at(homeZone)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(destination).at(homeZone)->getDistance() * OPERATIONAL_COST )/2
				-(amCostsMap.at(origin).at(homeZone)->getDistance() * OPERATIONAL_COST + pmCostsMap.at(origin).at(homeZone)->getDistance() * OPERATIONAL_COST)/2);
}

double StopModeDestinationParams::getCarCostERP(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getCarCostErp() + pmCostsMap.at(origin).at(destination)->getCarCostErp())/2
				+(amCostsMap.at(destination).at(homeZone)->getCarCostErp() + pmCostsMap.at(destination).at(homeZone)->getCarCostErp())/2
				-(amCostsMap.at(origin).at(homeZone)->getCarCostErp() + pmCostsMap.at(origin).at(homeZone)->getCarCostErp())/2);
}

double StopModeDestinationParams::getCostPublic(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubCost() + pmCostsMap.at(origin).at(destination)->getPubCost())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubCost() + pmCostsMap.at(destination).at(homeZone)->getPubCost())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubCost() + pmCostsMap.at(origin).at(homeZone)->getPubCost())/2);
}

double StopModeDestinationParams::getTT_CarIvt(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getCarIvt() + pmCostsMap.at(origin).at(destination)->getCarIvt())/2
				+(amCostsMap.at(destination).at(homeZone)->getCarIvt() + pmCostsMap.at(destination).at(homeZone)->getCarIvt())/2
				-(amCostsMap.at(origin).at(homeZone)->getCarIvt() + pmCostsMap.at(origin).at(homeZone)->getCarIvt())/2);
}

double StopModeDestinationParams::getTT_PubIvt(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubIvt() + pmCostsMap.at(origin).at(destination)->getPubIvt())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubIvt() + pmCostsMap.at(destination).at(homeZone)->getPubIvt())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubIvt() + pmCostsMap.at(origin).at(homeZone)->getPubIvt())/2);
}

double StopModeDestinationParams::getTT_PubOut(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return ((amCostsMap.at(origin).at(destination)->getPubOut() + pmCostsMap.at(origin).at(destination)->getPubOut())/2
				+(amCostsMap.at(destination).at(homeZone)->getPubOut() + pmCostsMap.at(destination).at(homeZone)->getPubOut())/2
				-(amCostsMap.at(origin).at(homeZone)->getPubOut() + pmCostsMap.at(origin).at(homeZone)->getPubOut())/2);
}

double StopModeDestinationParams::getWalkDistanceFirst(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return (amCostsMap.at(origin).at(destination)->getDistance()
				+ amCostsMap.at(destination).at(homeZone)->getDistance()
				- amCostsMap.at(origin).at(homeZone)->getDistance());
}

double StopModeDestinationParams::getWalkDistanceSecond(int zone) const {
	int destination = zoneMap.at(zone)->getZoneCode();
	if(origin == destination || destination == homeZone || origin == homeZone) { return 0; }
	return (pmCostsMap.at(origin).at(destination)->getDistance()
				+ pmCostsMap.at(destination).at(homeZone)->getDistance()
				- pmCostsMap.at(origin).at(homeZone)->getDistance());
}

int StopModeDestinationParams::getCentralDummy(int zone) const {
	return zoneMap.at(zone)->getCentralDummy();
}

int StopModeDestinationParams::getCbdDummy(int zone) const {
	return zoneMap.at(zone)->getCbdDummy();
}

StopType StopModeDestinationParams::getTourPurpose() const {
	return purpose;
}

double StopModeDestinationParams::getShop(int zone) const {
	return zoneMap.at(zone)->getShop();
}

double StopModeDestinationParams::getEmployment(int zone) const {
	return zoneMap.at(zone)->getEmployment();
}

double StopModeDestinationParams::getPopulation(int zone) const {
	return zoneMap.at(zone)->getPopulation();
}

double StopModeDestinationParams::getArea(int zone) const {
	return zoneMap.at(zone)->getArea();
}

int StopModeDestinationParams::isAvailable_IMD(int choiceId) const {
	/* 1. if the destination == origin, the destination is not available.
	 * 2. public bus, private bus and MRT/LRT are only available if AM[(origin,destination)][’pub_ivt’]>0 and PM[(destination,origin)][’pub_ivt’]>0
	 * 3. shared2, shared3+, taxi and motorcycle are available to all.
	 * 4. Walk is only avaiable if (AM[(origin,destination)][’distance’]<=2 and PM[(destination,origin)][’distance’]<=2)
	 * 5. drive alone is available when for the agent, has_driving_license * one_plus_car == True
	 */
	if (choiceId < 1 || choiceId > 9828) {
		throw std::runtime_error("isAvailable()::invalid choice id for mode-destination model");
	}
	int numZones = zoneMap.size();
	int zoneId = choiceId % numZones;
	if(zoneId == 0) { zoneId = numZones; } // zoneId will become zero for the last zone
	int destination = zoneMap.at(zoneId)->getZoneCode();

	if (origin == destination) { return 0; } // the destination same as origin is not available

	// check if destination is unavailable
	OD_Pair orgDest = OD_Pair(origin, destination);
	OD_Pair orgHome = OD_Pair(origin, homeZone);
	OD_Pair destHome = OD_Pair(destination, homeZone);

	if(std::binary_search(unavailableODs.begin(), unavailableODs.end(), orgDest) ||
			std::binary_search(unavailableODs.begin(), unavailableODs.end(), orgHome) ||
			std::binary_search(unavailableODs.begin(), unavailableODs.end(), destHome)) { return 0; } // destination is unavailable due to lack of cost data

	// bus 1-1092; mrt 1093 - 2184; private bus 2185 - 3276; same result for the three modes
	if (choiceId <= 3 * numZones) {
		bool avail = (pmCostsMap.at(destination).at(origin)->getPubIvt() > 0
				&& amCostsMap.at(origin).at(destination)->getPubIvt() > 0);
		switch(tourMode) {
		case 1:
		case 2:
		case 3: return avail;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9: return 0;
		}
	}

	// drive1 3277 - 4368
	if (choiceId <= 4 * numZones) {
		switch(tourMode) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6: return driveAvailable;
		case 7:
		case 8:
		case 9: return 0;
		}
	}
	// share2 4369 - 5460
	if (choiceId <= 6 * numZones) {
		// share2 is available to all
		switch(tourMode) {
		case 1:
		case 2:
		case 3:
		case 5:
		case 6: return 1;
		case 4:
		case 7:
		case 8:
		case 9: return 0;
		}
	}

	// motor 6553 - 7644
	if (choiceId <= 7 * numZones) {
		// share3 is available to all
		switch(tourMode) {
		case 1:
		case 2:
		case 3:
		case 5:
		case 6:
		case 4:
		case 7:
		case 9: return 1;
		case 8: return 0;
		}
	}
	// walk 7645 - 8736
	if (choiceId <= 8 * numZones) {
		return (amCostsMap.at(origin).at(destination)->getDistance() <= MAX_WALKING_DISTANCE
				&& pmCostsMap.at(destination).at(origin)->getDistance() <= MAX_WALKING_DISTANCE);
	}
	// taxi 8737 - 9828
	if (choiceId <= 9 * numZones) {
		// taxi is available to all
		switch(tourMode) {
		case 1:
		case 2:
		case 3:
		case 5:
		case 6:
		case 4:
		case 9: return 1;
		case 7:
		case 8: return 0;
		}
	}
	return 0;
}

int StopModeDestinationParams::isFirstBound() const { return firstBound; }

int StopModeDestinationParams::isSecondBound() const
{
	return !firstBound;
}
