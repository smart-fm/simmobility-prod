//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/*
 * ResidentialStatusDao.cpp
 *
 *  Created on: 4 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "ResidentialStatusDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ResidentialStatusDao::ResidentialStatusDao(DB_Connection& connection): SqlAbstractDao<ResidentialStatus>( connection, DB_TABLE_RESIDENTIAL_STATUS,DB_INSERT_RESIDENTIAL_STATUS,
																										  DB_UPDATE_RESIDENTIAL_STATUS, DB_DELETE_RESIDENTIAL_STATUS,
																										  DB_GETALL_RESIDENTIAL_STATUS, DB_GETBYID_RESIDENTIAL_STATUS){}


ResidentialStatusDao::~ResidentialStatusDao() {}


void ResidentialStatusDao::fromRow(Row& result, ResidentialStatus& outObj)
{
	outObj.id 	= result.get<BigSerial>( 	"id",   INVALID_ID	);
	outObj.name = result.get<std::string>(  "name", EMPTY_STR	);
}

void ResidentialStatusDao::toRow(ResidentialStatus& data, Parameters& outParams, bool update) {}

