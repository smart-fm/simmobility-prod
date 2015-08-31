//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * HitsIndividualLogsumDao.cpp
 *
 *  Created on: 17 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "HitsIndividualLogsumDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HitsIndividualLogsumDao::HitsIndividualLogsumDao(DB_Connection& connection): SqlAbstractDao<HitsIndividualLogsum>( connection, DB_TABLE_HITSINDIVIDUALLOGSUM, "", "", "", DB_GETALL_HITSINDIVIDUALLOGSUM, DB_GETBYID_HITSINDIVIDUALLOGSUM){}

HitsIndividualLogsumDao::~HitsIndividualLogsumDao(){}

void HitsIndividualLogsumDao::fromRow(Row& result, HitsIndividualLogsum& outObj)
{
	outObj.id			= result.get<int>( "id", 0);
    outObj.hitsId		= result.get<std::string>( "H1_HHID", "");
    outObj.paxId 		= result.get<int>( "PAX_ID", 0);
    outObj.homePostcode	= result.get<int>( "home_postcode", 0);
    outObj.homeTaz		= result.get<int>( "home_taz", 0);
    outObj.workPostcode	= result.get<int>( "work_postcode", 0);
    outObj.workTaz		= result.get<int>( "work_taz", 0);
    outObj.cars			= result.get<int>( "cars", 0);
}

void HitsIndividualLogsumDao::toRow(HitsIndividualLogsum& data, Parameters& outParams, bool update) {}
