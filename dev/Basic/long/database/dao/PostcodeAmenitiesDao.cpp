//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeAmenitiesDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:59 PM
 */

#include "PostcodeAmenitiesDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PostcodeAmenitiesDao::PostcodeAmenitiesDao(DB_Connection& connection): SqlAbstractDao<PostcodeAmenities>(connection, "","", "", "", "SELECT * FROM " + connection.getSchema()+"postcode_amenities", "")
{}

PostcodeAmenitiesDao::~PostcodeAmenitiesDao() {}

void PostcodeAmenitiesDao::fromRow(Row& result, PostcodeAmenities& outObj)
{
	outObj.addressId = result.get<BigSerial>("address_id", 0);
	outObj.tazId = result.get<BigSerial>("taz_id", 0);
    outObj.postcode = result.get<std::string>(DB_FIELD_POSTCODE, EMPTY_STR);
    outObj.mrtStation = result.get<std::string>(DB_FIELD_MRT_STATION, EMPTY_STR);
    outObj.distanceToMRT = result.get<double>(DB_FIELD_DISTANCE_TO_MRT, 0);
    outObj.distanceToBus = result.get<double>(DB_FIELD_DISTANCE_TO_BUS, 0);
    outObj.distanceToExpress = result.get<double>(DB_FIELD_DISTANCE_TO_EXPRESS, 0);
    outObj.distanceToPMS30 = result.get<double>(DB_FIELD_DISTANCE_TO_PMS30, 0);
    outObj.distanceToCBD = result.get<double>(DB_FIELD_DISTANCE_TO_CBD, 0);
    outObj.distanceToMall = result.get<double>(DB_FIELD_DISTANCE_TO_MALL, 0);
    outObj.mrt_200m = result.get<int>(DB_FIELD_MRT_200M, false);
    outObj.mrt_400m = result.get<int>(DB_FIELD_MRT_400M, false);
    outObj.express_200m = result.get<int>(DB_FIELD_EXPRESS_200M, false);
    outObj.bus_200m = result.get<int>(DB_FIELD_BUS_200M, false);
    outObj.bus_400m = result.get<int>(DB_FIELD_BUS_400M, false);
    outObj.pms_1km = result.get<int>(DB_FIELD_PMS_1KM, false);
}

void PostcodeAmenitiesDao::toRow(PostcodeAmenities& data, Parameters& outParams, bool update) {}
