/*
 * DistanceMRTDao.cpp
 *
 *  Created on: Jun 2, 2015
 *      Author: gishara
 */

#include "DistanceMRTDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

DistanceMRTDao::DistanceMRTDao(DB_Connection& connection)
: SqlAbstractDao<DistanceMRT>(connection, EMPTY_STR, EMPTY_STR, EMPTY_STR, EMPTY_STR, DB_GETALL_DIST_MRT, EMPTY_STR)
{}

DistanceMRTDao::~DistanceMRTDao() {
}

void DistanceMRTDao::fromRow(Row& result, DistanceMRT& outObj)
{
    outObj.houseHoldId = result.get<BigSerial>("household_id", INVALID_ID);
    outObj.distanceMrt = result.get<double>("dist_mrt", 0.0);
}

void DistanceMRTDao::toRow(DistanceMRT& data, Parameters& outParams, bool update) {}


