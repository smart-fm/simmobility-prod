/*
 * BuildingAvgAgeDao.cpp
 *
 *  Created on: 23 Feb 2016
 *      Author: gishara
 */
#include "BuildingAvgAgePerParcelDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

BuildingAvgAgePerParcelDao::BuildingAvgAgePerParcelDao(DB_Connection& connection): SqlAbstractDao<BuildingAvgAgePerParcel>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"fm_building_avg_age", "") {}

BuildingAvgAgePerParcelDao::~BuildingAvgAgePerParcelDao() {}

void BuildingAvgAgePerParcelDao::fromRow(Row& result, BuildingAvgAgePerParcel& outObj)
{
    outObj.fmParcelId  = result.get<BigSerial>("fm_parcel_id",INVALID_ID);
    outObj.age = result.get<int>("avg_age",0);

}

void BuildingAvgAgePerParcelDao::toRow(BuildingAvgAgePerParcel& data, Parameters& outParams, bool update)
{

}
