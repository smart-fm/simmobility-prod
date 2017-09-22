//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * OwnerTenantMovingRateDao.cpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/TenureTransitionRateDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TenureTransitionRateDao::TenureTransitionRateDao(DB_Connection& connection): SqlAbstractDao<TenureTransitionRate>(  connection, "", "", "", "",
																													"SELECT * FROM " + connection.getSchema()+"tenure_transition_rate",
																													""){}


TenureTransitionRateDao::~TenureTransitionRateDao(){}

void TenureTransitionRateDao::fromRow(Row& result, TenureTransitionRate& outObj)
{
    outObj.id				= result.get<BigSerial>( "id", INVALID_ID);
    outObj.ageGroup 		= result.get<std::string>( "age_group", "");
    outObj.currentStatus	= result.get<std::string>( "current_status", "");
    outObj.futureStatus		= result.get<std::string>( "future_status", "");
    outObj.rate				= result.get<double>( "rate", 0);
}

void TenureTransitionRateDao::toRow(TenureTransitionRate& data, Parameters& outParams, bool update) {}


