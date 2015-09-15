//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
* TazLevelLandPriceDao.cpp
 *
 *  Created on: Sep 3, 2015
 *      Author: gishara
 */

#include <database/dao/TazLevelLandPriceDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TazLevelLandPriceDao::TazLevelLandPriceDao(DB_Connection& connection): SqlAbstractDao<TazLevelLandPrice>(connection, DB_TABLE_TAZ_LEVEL_LAND_PRICE,EMPTY_STR, EMPTY_STR, EMPTY_STR,DB_GETALL_TAZ_LEVEL_LAND_PRICES, EMPTY_STR){}

TazLevelLandPriceDao::~TazLevelLandPriceDao() {}

void TazLevelLandPriceDao::fromRow(Row& result, TazLevelLandPrice& outObj)
{
    outObj.tazId = result.get<BigSerial>("taz_id", INVALID_ID);
    outObj.landValue = result.get<double>("land_value",0.0);
}

void TazLevelLandPriceDao::toRow(TazLevelLandPrice& data, Parameters& outParams, bool update) {}



