//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * OwnerTenantMovingRateDao.cpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/OwnerTenantMovingRateDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

OwnerTenantMovingRateDao::OwnerTenantMovingRateDao(DB_Connection& connection): SqlAbstractDao<OwnerTenantMovingRate>( connection, DB_TABLE_OWNERTENANTMOVINGRATE, "", "", "",
																				DB_GETALL_OWNERTENANTMOVINGRATE, DB_GETBYID_OWNERTENANTMOVINGRATE){}

OwnerTenantMovingRateDao::~OwnerTenantMovingRateDao(){}

void OwnerTenantMovingRateDao::fromRow(Row& result, OwnerTenantMovingRate& outObj)
{
    outObj.id				= result.get<BigSerial>( "id", INVALID_ID);
    outObj.ageCategory 		= result.get<int>( "age_category_id", 0);
    outObj.ownerPopulation		= result.get<int>( "owner_population", 0);
    outObj.tenantPopulation	= result.get<int>( "tenant_population", 0);
    outObj.ownerMovingPercentage	= result.get<int>( "owner_moving_percentage", 0);
    outObj.tenantMovingPercentage	= result.get<int>( "tenant_moving_percentage", 0);
}

void OwnerTenantMovingRateDao::toRow(OwnerTenantMovingRate& data, Parameters& outParams, bool update) {}


