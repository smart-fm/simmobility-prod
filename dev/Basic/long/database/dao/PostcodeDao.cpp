//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:59 PM
 */

#include "PostcodeDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PostcodeDao::PostcodeDao(DB_Connection& connection): SqlAbstractDao<Postcode>(connection, 	"",	"",	"",	"",	"SELECT * FROM " + connection.getSchema() + "sla_addresses", "" )
{}

PostcodeDao::~PostcodeDao() {}

void PostcodeDao::fromRow(Row& result, Postcode& outObj)
{
    outObj.address_id 		= result.get<BigSerial>(	"address_id", 		INVALID_ID);
    outObj.sla_postcode 	= result.get<std::string>(	"sla_postcode", 	EMPTY_STR);
    outObj.taz_id 			= result.get<BigSerial>(	"taz_id", 			INVALID_ID);
    outObj.longitude 		= result.get<double>(		"y_coord", 		.0);
    outObj.latitude 		= result.get<double>(		"x_coord", 		.0);
    outObj.primary_postcode = result.get<int>(			"primary_postcode",  0);
}

void PostcodeDao::toRow(Postcode& data, Parameters& outParams, bool update) {}
