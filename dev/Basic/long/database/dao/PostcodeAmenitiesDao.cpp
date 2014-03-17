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

PostcodeAmenitiesDao::PostcodeAmenitiesDao(DB_Connection& connection)
: SqlAbstractDao<PostcodeAmenities>(connection, DB_TABLE_POSTCODE_AMENITIES,
DB_INSERT_POSTCODE_AMENITIES, DB_UPDATE_POSTCODE_AMENITIES, DB_DELETE_POSTCODE_AMENITIES,
DB_GETALL_POSTCODE_AMENITIES, DB_GETBYID_POSTCODE_AMENITIES) {
}

PostcodeAmenitiesDao::~PostcodeAmenitiesDao() {
}

void PostcodeAmenitiesDao::fromRow(Row& result, PostcodeAmenities& outObj) {
    outObj.postcode = result.get<std::string>(DB_FIELD_POSTCODE, EMPTY_STR);
    outObj.buildingName = result.get<std::string>(DB_FIELD_BUILDING_NAME, EMPTY_STR);
    outObj.roadName = result.get<std::string>(DB_FIELD_ROAD_NAME, EMPTY_STR);
    outObj.unitBlock = result.get<std::string>(DB_FIELD_UNIT_BLOCK, EMPTY_STR);
    outObj.mtzNumber = result.get<std::string>(DB_FIELD_MTZ_NUMBER, EMPTY_STR);
    outObj.mrtStation = result.get<std::string>(DB_FIELD_MRT_STATION, EMPTY_STR);
    outObj.distanceToMRT = result.get<double>(DB_FIELD_DISTANCE_TO_MRT, 0);
    outObj.distanceToBus = result.get<double>(DB_FIELD_DISTANCE_TO_BUS, 0);
    outObj.distanceToExpress = result.get<double>(DB_FIELD_DISTANCE_TO_EXPRESS, 0);
    outObj.distanceToPMS30 = result.get<double>(DB_FIELD_DISTANCE_TO_PMS30, 0);
    outObj.distanceToCBD = result.get<double>(DB_FIELD_DISTANCE_TO_CBD, 0);
    outObj.distanceToMall = result.get<double>(DB_FIELD_DISTANCE_TO_MALL, 0);
    outObj.distanceToJob = result.get<double>(DB_FIELD_DISTANCE_TO_JOB, 0);
    outObj.mrt_200m = result.get<int>(DB_FIELD_MRT_200M, false);
    outObj.mrt_400m = result.get<int>(DB_FIELD_MRT_400M, false);
    outObj.express_200m = result.get<int>(DB_FIELD_EXPRESS_200M, false);
    outObj.bus_200m = result.get<int>(DB_FIELD_BUS_200M, false);
    outObj.bus_400m = result.get<int>(DB_FIELD_BUS_400M, false);
    outObj.pms_1km = result.get<int>(DB_FIELD_PMS_1KM, false);
    outObj.apartment = result.get<int>(DB_FIELD_APARTMENT, false);
    outObj.condo = result.get<int>(DB_FIELD_CONDO, false);
    outObj.terrace = result.get<int>(DB_FIELD_TERRACE, false);
    outObj.semi = result.get<int>(DB_FIELD_SEMI, false);
    outObj.detached = result.get<int>(DB_FIELD_DETACHED, false);
    outObj.ec = result.get<int>(DB_FIELD_EC, false);
    outObj._private = result.get<int>(DB_FIELD_PRIVATE, false);
    outObj.hdb = result.get<int>(DB_FIELD_HDB, false);
}

void PostcodeAmenitiesDao::toRow(PostcodeAmenities& data, Parameters& outParams, bool update) {
}
