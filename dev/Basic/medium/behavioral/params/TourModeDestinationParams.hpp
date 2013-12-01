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
#include <string>
#include "behavioral/params/ZoneCostParams.hpp"
#include "behavioral/PredayClasses.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "database/ZoneCostMongoDao.hpp"
#include "mongo/client/dbclient.h"

namespace sim_mob {
namespace medium {

class TourModeDestinationParams {
public:
	TourModeDestinationParams(boost::unordered_map<int, ZoneParams>& zoneMap, PersonParams& personParams, StopType tourType)
	: zoneMap(zoneMap), origin(personParams.getHomeLocation()), tourType(tourType), drive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal())
	{
		std::string amCostsCollName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("AMCosts");
		std::string pmCostsCollName = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo").collectionName.at("PMCosts");
		Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
		std::string emptyString;
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		CostMongoDao amCostDao(dbConfig, db.dbName, amCostsCollName);
		CostMongoDao pmCostDao(dbConfig, db.dbName, pmCostsCollName);
		mongo::Query amQuery= QUERY("origin" << origin);
		mongo::Query pmQuery = QUERY("destin" << origin);
		amCostDao.getMultiple(amQuery, amCostsMap);
		pmCostDao.getMultiple(pmQuery, pmCostsMap);
	}

	virtual ~TourModeDestinationParams() {
		for(boost::unordered_map<const std::string, CostParams*>::iterator i = amCostsMap.begin(); i!=amCostsMap.end(); i++) {
			safe_delete_item(i->second);
		}
		amCostsMap.clear();
		for(boost::unordered_map<const std::string, CostParams*>::iterator i = pmCostsMap.begin(); i!=pmCostsMap.end(); i++) {
			safe_delete_item(i->second);
		}
		pmCostsMap.clear();
	}

	StopType getTourType() const {
		return tourType;
	}

	double getCostPublicFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getPubCost();
	}

	double getCostPublicSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getPubCost();
	}

	double getCostCarERPFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getCarCostErp();
	}

	double getCostCarERPSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getCarCostErp();
	}

	double getCostCarOPFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return (amCostsMap.at(orgDestStr)->getDistance() * 0.147);
	}

	double getCostCarOPSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return (pmCostsMap.at(destOrgStr)->getDistance() * 0.147);
	}

	double getCostCarParking(int zoneId) {
		return (8*zoneMap.at(zoneId).getParkingRate());
	}

	double getWalkDistance1(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getPubWalkt();
	}

	double getWalkDistance2(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getPubWalkt();
	}

	int getCentralDummy(int zoneId) {
		return zoneMap.at(zoneId).getCentralDummy();
	}

	double getTT_PublicIvtFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getPubIvt();
	}

	double getTT_PublicIvtSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getPubIvt();
	}

	double getTT_CarIvtFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getCarIvt();
	}

	double getTT_CarIvtSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getCarIvt();
	}

	double getTT_PublicOutFirst(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		return amCostsMap.at(orgDestStr)->getPubOut();
	}

	double getTT_PublicOutSecond(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return pmCostsMap.at(destOrgStr)->getPubOut();
	}

	double getAvgTransferNumber(int zoneId) {
		int destination = zoneMap.at(zoneId).getZoneCode();
		if (origin == destination) { return 0; }
		const std::string& orgDestStr = getOrgDestString(origin, destination);
		const std::string& destOrgStr = getOrgDestString(destination, origin);
		return (amCostsMap.at(orgDestStr)->getAvgTransfer() + pmCostsMap.at(destOrgStr)->getAvgTransfer())/2;
	}

	double getEmployment(int zoneId) {
		return zoneMap.at(zoneId).getEmployment();
	}

	double getArea(int zoneId) {
		return zoneMap.at(zoneId).getArea();
	}

	double getPopulation(int zoneId) {
		return zoneMap.at(zoneId).getPopulation();
	}

	double getShop(int zoneId) {
		return zoneMap.at(zoneId).getShop();
	}

	void setDrive1Available(bool drive1Available) {
		this->drive1Available = drive1Available;
	}

	int isAvailable(int choiceId) {
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
		int destination = zoneMap.at(zoneId).getZoneCode();
		// the destination same as origin is not available
		if(origin == destination) {
			return 0;
		}
		// bus 1-1092; mrt 1093 - 2184; private bus 2185 - 3276; same result for the three modes
		if(choiceId <= 3*numZones) {
			const std::string& orgDestStr = getOrgDestString(origin, destination);
			const std::string& destOrgStr = getOrgDestString(destination, origin);
			return (pmCostsMap.at(destOrgStr)->getPubIvt()>0 && amCostsMap.at(orgDestStr)->getPubIvt()>0);
		}
		// drive1 3277 - 4368
		if(choiceId <= 4*numZones) {
			return drive1Available;
		}
		// share2 4369 - 5460
		if(choiceId <= 5*numZones) { // share2 is available to all
			return 1;
		}
		// share3 5461 - 6552
		if(choiceId <= 6*numZones) { // share3 is available to all
			return 1;
		}
		// motor 6553 - 7644
		if(choiceId <= 7*numZones) { // share3 is available to all
			return 1;
		}
		// walk 7645 - 8736
		if(choiceId <= 8*numZones) {
			const std::string& orgDestStr = getOrgDestString(origin, destination);
			const std::string& destOrgStr = getOrgDestString(destination, origin);
			return (amCostsMap.at(orgDestStr)->getDistance() <= 2 && pmCostsMap.at(destOrgStr)->getDistance() <= 2);
		}
		// taxi 8737 - 9828
		if(choiceId <= 9*numZones) { // taxi is available to all
			return 1;
		}
		return 0;
	}

	int getMode(int choice) {
		if (choice < 1 || choice > 9828) {
			throw std::runtime_error("getMode()::invalid choice id for mode-destination model");
		}
		return (choice/zoneMap.size() + 1);
	}

	int getDestination(int choice) {
		if (choice < 1 || choice > 9828) {
			throw std::runtime_error("getDestination()::invalid choice id for mode-destination model");
		}
		return (choice % zoneMap.size());
	}

private:
	const std::string getOrgDestString(int org, int dest) const {
		std::stringstream ss;
		ss << org << "," << dest;
		return ss.str();
	}

	int origin;
	StopType tourType;
	bool drive1Available;
	boost::unordered_map<int, ZoneParams>& zoneMap;
	boost::unordered_map<const std::string, CostParams*> amCostsMap;
	boost::unordered_map<const std::string, CostParams*> pmCostsMap;

};
} // end namespace medium
} // end namespace sim_mob
