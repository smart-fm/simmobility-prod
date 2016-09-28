//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * LogsumMtzV2Dao.cpp
 *
 *  Created on: 27 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include "LogsumMtzV2Dao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

LogsumMtzV2Dao::LogsumMtzV2Dao(DB_Connection& connection): SqlAbstractDao<LogsumMtzV2>( connection, DB_TABLE_LOGSUMMTZV2, "", "", "", DB_GETALL_LOGSUMMTZV2, DB_GETBYID_LOGSUMMTZV2){}

LogsumMtzV2Dao::~LogsumMtzV2Dao(){}

void LogsumMtzV2Dao::fromRow(Row& result, LogsumMtzV2& outObj)
{
    outObj.taz_id				= result.get<BigSerial>( "taz_id", 0);
    outObj.name				= result.get<BigSerial>( "name", 0);
    outObj.logsumAvg		= result.get<double>( "avg_logsum", .0);
    outObj.logsumWeighted	= result.get<double>( "logsum_weighted", .0);
}

void LogsumMtzV2Dao::toRow(LogsumMtzV2& data, Parameters& outParams, bool update) {}
