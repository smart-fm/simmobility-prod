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
	outParam.setShop(document.getField(MONGO_FIELD_ZONE_SHOPS).Number());
	outParam.setParkingRate(document.getField(MONGO_FIELD_ZONE_PARKING_RATE).Number());
	outParam.setResidentWorkers(document.getField(MONGO_FIELD_ZONE_RESIDENT_WORKERS).Number());
	outParam.setCentralDummy(document.getField(MONGO_FIELD_ZONE_CENTRAL_ZONE).Number() > 0);
	outParam.setEmployment(document.getField(MONGO_FIELD_ZONE_EMPLOYMENT).Number());
	outParam.setPopulation(document.getField(MONGO_FIELD_ZONE_POPULATION).Number());
	outParam.setArea(document.getField(MONGO_FIELD_ZONE_AREA).Number());
	outParam.setTotalEnrollment(document.getField(MONGO_FIELD_ZONE_TOTAL_ENROLLMENT).Number());
	outParam.setResidentStudents(document.getField(MONGO_FIELD_ZONE_RESIDENT_STUDENTS).Number());
	if(document.hasElement(MONGO_FIELD_ZONE_CBD_ZONE)) { outParam.setCbdDummy(document.getField(MONGO_FIELD_ZONE_CBD_ZONE).Number()); }
	else { outParam.setCbdDummy(false); }
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
	outParam.setPubWtt(document.getField(MONGO_FIELD_COST_PUB_WTT).Number());
	outParam.setCarIvt(document.getField(MONGO_FIELD_COST_CAR_IVT).Number());
	outParam.setPubOut(document.getField(MONGO_FIELD_COST_PUB_OUT).Number());
	outParam.setPubWalkt(document.getField(MONGO_FIELD_COST_PUB_WALKT).Number());
	outParam.setDistance(document.getField(MONGO_FIELD_COST_DISTANCE).Number());
	outParam.setCarCostErp(document.getField(MONGO_FIELD_COST_CAR_ERP).Number());
	outParam.setPubIvt(document.getField(MONGO_FIELD_COST_PUB_IVT).Number());
	outParam.setAvgTransfer(document.getField(MONGO_FIELD_COST_AVG_TRANSFER).Number());
	outParam.setPubCost(document.getField(MONGO_FIELD_COST_PUB_COST).Number());
}

sim_mob::medium::ZoneNodeMappingDao::ZoneNodeMappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::ZoneNodeMappingDao::~ZoneNodeMappingDao()
{}

bool sim_mob::medium::ZoneNodeMappingDao::getAll(boost::unordered_map<int, std::vector<ZoneNodeParams*> >& outList) {
	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, mongo::BSONObj());
	while(cursor->more()) {
		ZoneNodeParams* zoneNodeParams = new ZoneNodeParams();
		fromRow(cursor->next(), *zoneNodeParams);
		outList[zoneNodeParams->getZone()].push_back(zoneNodeParams);
	}
	return true;
}

void sim_mob::medium::ZoneNodeMappingDao::fromRow(mongo::BSONObj document, ZoneNodeParams& outParam) {
	outParam.setZone(document.getField(MONGO_FIELD_MTZ).Int());
	switch (document.getField(MONGO_FIELD_NODE_ID).type()) {
	// the node id is stored in either integer format or long format in mongo db
	case mongo::NumberInt:
	{
		outParam.setAimsunNodeId(document.getField(MONGO_FIELD_NODE_ID).Int());
		break;
	}
	case mongo::NumberLong:
	{
		outParam.setAimsunNodeId(document.getField(MONGO_FIELD_NODE_ID).Long());
		break;
	}
	default:
	{
		Print() << "zoneNodeDoc.getField(MONGO_FIELD_NODE_ID).type() = " << document.getField(MONGO_FIELD_NODE_ID).type() << std::endl;
	}
	}
	outParam.setSourceNode(document.getField(MONGO_FIELD_SOURCE_NODE).Number());
	outParam.setSinkNode(document.getField(MONGO_FIELD_SINK_NODE).Number());
}

sim_mob::medium::MTZ12_MTZ08_MappingDao::MTZ12_MTZ08_MappingDao(db::DB_Config& dbConfig, const std::string& database, const std::string& collection)
: db::MongoDao(dbConfig, database, collection)
{}

sim_mob::medium::MTZ12_MTZ08_MappingDao::~MTZ12_MTZ08_MappingDao()
{}

bool sim_mob::medium::MTZ12_MTZ08_MappingDao::getAll(std::map<int, int >& outList)
{
	std::auto_ptr<mongo::DBClientCursor> cursor = connection.getSession<mongo::DBClientConnection>().query(collectionName, mongo::BSONObj());
	while(cursor->more())
	{
		mongo::BSONObj document = cursor->next();
		outList[document.getField(MONGO_FIELD_ID).Int()] = document.getField(MONGO_FIELD_MTZ1092).Int();
	}
	return true;
}
