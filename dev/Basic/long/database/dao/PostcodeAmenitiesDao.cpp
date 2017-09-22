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
    outObj.postcode = result.get<std::string>("sla_postcode", EMPTY_STR);
    outObj.mrtStation = result.get<std::string>("mrt_station", EMPTY_STR);
    outObj.distanceToMRT = result.get<double>("distance_mrt", 0);
    outObj.distanceToBus = result.get<double>("distance_bus", 0);
    outObj.distanceToExpress = result.get<double>("distance_express", 0);
    outObj.distanceToPMS30 = result.get<double>("distance_pms30", 0);
    outObj.distanceToCBD = result.get<double>("distance_cbd", 0);
    outObj.distanceToMall = result.get<double>("distance_mall", 0);
    outObj.mrt_200m = result.get<int>("mrt_200m", false);
    outObj.mrt_400m = result.get<int>("mrt_400m", false);
    outObj.express_200m = result.get<int>("express_200m", false);
    outObj.bus_200m = result.get<int>("bus_200m", false);
    outObj.bus_400m = result.get<int>("bus_400m", false);
    outObj.pms_1km = result.get<int>("pms_1km", false);

}

void PostcodeAmenitiesDao::toRow(PostcodeAmenities& data, Parameters& outParams, bool update) {}
