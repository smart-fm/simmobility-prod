/*
 * ParcelAmenitiesDao.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: gishara
 */
#include "ParcelAmenitiesDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ParcelAmenitiesDao::ParcelAmenitiesDao(DB_Connection& connection): SqlAbstractDao<ParcelAmenities>(connection, DB_TABLE_PARCEL_AMENITIES,EMPTY_STR, EMPTY_STR, EMPTY_STR,DB_GETALL_PARCEL_AMENITIES, EMPTY_STR)
{}

ParcelAmenitiesDao::~ParcelAmenitiesDao() {}

void ParcelAmenitiesDao::fromRow(Row& result, ParcelAmenities& outObj)
{
    outObj.fmParcelId = result.get<BigSerial>("fm_parcel_id",INVALID_ID);
    outObj.nearestMRT = result.get<std::string>("nearest_mrt",EMPTY_STR);
    outObj.distanceToMRT = result.get<double>("distance_mrt", 0);
    outObj.distanceToBus = result.get<double>("distance_bus", 0);
    outObj.distanceToExpress = result.get<double>("distance_express", 0);
    outObj.distanceToPMS30 = result.get<double>("distance_pms30", 0);
    outObj.distanceToCBD = result.get<double>("distance_cbd", 0);
    outObj.distanceToMall = result.get<double>("distance_mall", 0);
    outObj.distanceToJob = result.get<double>("accessibility_job", 0);
    outObj.mrt_200m = result.get<int>("mrt_200m", false);
    outObj.mrt_400m = result.get<int>("mrt_400m", false);
    outObj.express_200m = result.get<int>("express_200m", false);
    outObj.bus_200m = result.get<int>("bus_200m", false);
    outObj.bus_400m = result.get<int>("bus_400m", false);
    outObj.pms_1km = result.get<int>("pms_1km", false);
}

void ParcelAmenitiesDao::toRow(ParcelAmenities& data, Parameters& outParams, bool update) {}




