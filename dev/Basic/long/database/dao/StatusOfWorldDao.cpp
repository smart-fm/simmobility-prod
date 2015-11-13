/*
 * StatusOfWorldDao.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */
#include "StatusOfWorldDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

StatusOfWorldDao::StatusOfWorldDao(DB_Connection& connection): SqlAbstractDao<StatusOfWorld>(connection, DB_TABLE_STATUS_OF_WORLD, DB_INSERT_STATUS_OF_WORLD,EMPTY_STR, EMPTY_STR, DB_GETALL_STAUS_OF_WORLD, EMPTY_STR) {
}

StatusOfWorldDao::~StatusOfWorldDao() {
}

void StatusOfWorldDao::fromRow(Row& result, StatusOfWorld& outObj)
{
	outObj.simVersionId = result.get<BigSerial>("sim_version_id", INVALID_ID);
	outObj.postcode = result.get<BigSerial>("postcode", INVALID_ID);
	outObj.buildingId = result.get<BigSerial>("building_id", INVALID_ID);
	outObj.unitId = result.get<BigSerial>("unit_id",INVALID_ID);
	outObj.projectId = result.get<BigSerial>("project_id",INVALID_ID);
}

void StatusOfWorldDao::toRow(StatusOfWorld& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getSimVersionId());
	outParams.push_back(data.getPostcode());
	outParams.push_back(data.getBuildingId());
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getProjectId());
}




