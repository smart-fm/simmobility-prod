/*
 * LogSumVehicleOwnershipDao.cpp
 *
 *  Created on: May 21, 2015
 *      Author: gishara
 */

#include "LogSumVehicleOwnershipDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

LogSumVehicleOwnershipDao::LogSumVehicleOwnershipDao(DB_Connection& connection): SqlAbstractDao<LogSumVehicleOwnership>( connection, "", "", "", "",
																														 "SELECT * FROM " + connection.getSchema()+"getCarOwnershipLogsumsPerHH()", "" ) {}


LogSumVehicleOwnershipDao::~LogSumVehicleOwnershipDao() {}

void LogSumVehicleOwnershipDao::fromRow(Row& result, LogSumVehicleOwnership& outObj)
{
    outObj.householdId 		= result.get<BigSerial>("id",INVALID_ID);
    outObj.logsum 		= result.get<double>("avg",0.0);
}

void LogSumVehicleOwnershipDao::toRow(LogSumVehicleOwnership& data, Parameters& outParams, bool update) {}




