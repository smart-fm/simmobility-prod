//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZoneMongoDao.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: Harish Loganathan
 */

#include "ZoneCostMongoDao.hpp"
#include "DatabaseHelper.hpp"
#include "mongo/client/dbclient.h"

sim_mob::medium::ZoneMongoDao::ZoneMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::ZoneMongoDao::~ZoneMongoDao()
{}

void sim_mob::medium::ZoneMongoDao::fromRow(mongo::BSONObj document, ZoneParams& outParam) {
	outParam.setZoneId(document.getField(MONGO_FIELD_ZONE_ID).Int());
	outParam.setZoneCode(document.getField(MONGO_FIELD_ZONE_CODE).Int());
	outParam.setShop(document.getField(MONGO_FIELD_ZONE_SHOPS).Double());
	outParam.setParkingRate(document.getField(MONGO_FIELD_ZONE_PARKING_RATE).Double());
	outParam.setResidentWorkers(document.getField(MONGO_FIELD_ZONE_RESIDENT_WORKERS).Double());
	outParam.setCentralDummy(document.getField(MONGO_FIELD_ZONE_CENTRAL_ZONE).Double() > 0);
	outParam.setEmployment(document.getField(MONGO_FIELD_ZONE_EMPLOYMENT).Double());
	outParam.setPopulation(document.getField(MONGO_FIELD_ZONE_POPULATION).Double());
	outParam.setArea(document.getField(MONGO_FIELD_ZONE_AREA).Double());
	outParam.setTotalEnrollment(document.getField(MONGO_FIELD_ZONE_TOTAL_ENROLLMENT).Double());
	outParam.setResidentStudents(document.getField(MONGO_FIELD_ZONE_RESIDENT_STUDENTS).Double());
}

sim_mob::medium::CostMongoDao::CostMongoDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::CostMongoDao::~CostMongoDao()
{}

void sim_mob::medium::CostMongoDao::fromRow(mongo::BSONObj document, CostParams& outParam) {
	outParam.setOriginZone(document.getField(MONGO_FIELD_COST_ORIGIN).Int());
	outParam.setDestinationZone(document.getField(MONGO_FIELD_COST_DESTINATION).Int());
	outParam.setOrgDest();
	outParam.setPubWtt(document.getField(MONGO_FIELD_COST_PUB_WTT).Double());
	outParam.setCarIvt(document.getField(MONGO_FIELD_COST_CAR_IVT).Double());
	outParam.setPubOut(document.getField(MONGO_FIELD_COST_PUB_OUT).Double());
	outParam.setPubWalkt(document.getField(MONGO_FIELD_COST_PUB_WALKT).Double());
	outParam.setDistance(document.getField(MONGO_FIELD_COST_DISTANCE).Double());
	outParam.setCarCostErp(document.getField(MONGO_FIELD_COST_CAR_ERP).Double());
	outParam.setPubIvt(document.getField(MONGO_FIELD_COST_PUB_IVT).Double());
	outParam.setAvgTransfer(document.getField(MONGO_FIELD_COST_AVG_TRANSFER).Double());
	outParam.setPubCost(document.getField(MONGO_FIELD_COST_PUB_COST).Double());
}

sim_mob::medium::ZoneNodeMappingDao::ZoneNodeMappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::ZoneNodeMappingDao::~ZoneNodeMappingDao()
{}

bool sim_mob::medium::ZoneNodeMappingDao::getAll(boost::unordered_map<int, std::vector<long> >& outList) {
	int numZones = connection.getSession<mongo::DBClientConnection>().count(collectionName, mongo::BSONObj());
	for(int znid = 1; znid <= numZones; znid++) {
		mongo::Query queryObj = QUERY("Zone_ID" << znid);
    	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, queryObj);
    	while(cursor->more()) {
    		mongo::BSONObj zoneNodeDoc = cursor->next();
    		// the node id is stored in either integer format or long format in mongo db
    		switch (zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).type()) {
				case mongo::NumberInt:
				{
					outList[znid].push_back(zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).Int());
					break;
				}
				case mongo::NumberLong:
				{
					outList[znid].push_back(zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).Long());
					break;
				}
				default:
				{
					Print() << "zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).type() = " << zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).type() << std::endl;
				}
    		}
    	}
	}
	return true;
}
