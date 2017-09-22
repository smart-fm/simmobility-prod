/*
 * HHCoordinatesDao.cpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#include "HHCoordinatesDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HHCoordinatesDao::HHCoordinatesDao(DB_Connection& connection): SqlAbstractDao<HHCoordinates>( connection, "", "", "", "","SELECT * FROM " + connection.getSchema()+"getHouseholdCoordinates()", ""){}

HHCoordinatesDao::~HHCoordinatesDao(){}

void HHCoordinatesDao::fromRow(Row& result, HHCoordinates& outObj)
{
    outObj.houseHoldId	= result.get<BigSerial>( "id", INVALID_ID);
    outObj.centroidX	= result.get<double>( "centroid_x", 0);
    outObj.centroidY	= result.get<double>( "centroid_y", 0);

}

void HHCoordinatesDao::toRow(HHCoordinates& data, Parameters& outParams, bool update) {}




