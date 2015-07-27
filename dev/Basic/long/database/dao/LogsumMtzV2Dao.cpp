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
    outObj.taz				= result.get<int>( "taz", 0);
    outObj.v2				= result.get<double>( "v2", .0);
    outObj.logsum_mean		= result.get<double>( "logsum_mean", .0);
    outObj.logsum_sd		= result.get<double>( "logsum_sd", .0);
    outObj.logsum_max		= result.get<double>( "logsum_max", .0);
    outObj.logsum_min		= result.get<double>( "logsum_min", .0);
    outObj.logsum_total		= result.get<double>( "logsum_total", .0);
    outObj.factor_total		= result.get<double>( "factor_total", .0);
    outObj.logsum_weighted	= result.get<double>( "logsum_weighted", .0);
}

void LogsumMtzV2Dao::toRow(LogsumMtzV2& data, Parameters& outParams, bool update) {}
