//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TazLogsumWeightDao.cpp
 *
 *  Created on: 25 Jun, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/TazLogsumWeightDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TazLogsumWeightDao::TazLogsumWeightDao(DB_Connection& connection): SqlAbstractDao<TazLogsumWeight>(connection, DB_TABLE_TAZ_LOGUM_WEIGHT,EMPTY_STR, EMPTY_STR, EMPTY_STR, DB_GETALL_TAZ_LOGSUM_WEIGHTS, DB_GETBYID_TAZ_LOGSUM_WEIGHT){}

TazLogsumWeightDao::~TazLogsumWeightDao() {}

void TazLogsumWeightDao::fromRow(Row& result, TazLogsumWeight& outObj)
{
    outObj.groupLogsum = result.get<int>("group_logsum", 0);
    outObj.individualId = result.get<BigSerial>("individual_id", 0);
    outObj.weight = result.get<double>("weight", 0.0);

}

void TazLogsumWeightDao::toRow(TazLogsumWeight& data, Parameters& outParams, bool update) {}
