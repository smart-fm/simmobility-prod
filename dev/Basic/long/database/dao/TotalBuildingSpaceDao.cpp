/*
 * TotalBuildingSpaceDao.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: gishara
 */

#include "TotalBuildingSpaceDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TotalBuildingSpaceDao::TotalBuildingSpaceDao(DB_Connection& connection): SqlAbstractDao<TotalBuildingSpace>( connection, DB_GETALL_TOTAL_BUILDING_SPACE, EMPTY_STR, EMPTY_STR, EMPTY_STR,
		EMPTY_STR, EMPTY_STR ) {}

TotalBuildingSpaceDao::~TotalBuildingSpaceDao() {}

void TotalBuildingSpaceDao::fromRow(Row& result, TotalBuildingSpace& outObj)
{
    outObj.fmParcelId				= result.get<BigSerial>("fm_parcel_id",INVALID_ID);
    outObj.totalBuildingSpace		= result.get<double>("total_building_space",0);
}

void TotalBuildingSpaceDao::toRow(TotalBuildingSpace& data, Parameters& outParams, bool update) {}

std::vector<TotalBuildingSpace*> TotalBuildingSpaceDao::getBuildingSpaces()
{
	const std::string queryStr = DB_GETALL_TOTAL_BUILDING_SPACE;
	std::vector<TotalBuildingSpace*> totalBuildingSpaceList;
	getByQuery(queryStr,totalBuildingSpaceList);
	return totalBuildingSpaceList;
}




