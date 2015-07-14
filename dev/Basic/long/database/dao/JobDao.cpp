/*
 * JobDao.cpp
 *
 *  Created on: 23 Apr, 2015
 *      Author: chetan
 */

#include <database/dao/JobDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

JobDao::JobDao(DB_Connection& connection): SqlAbstractDao<Job>(connection, DB_TABLE_JOB, EMPTY_STR, EMPTY_STR, EMPTY_STR, DB_GETALL_JOB, DB_GETBYID_JOB)
{}

JobDao::~JobDao() {}

void JobDao::fromRow(Row& result, Job& outObj)
{
    outObj.id = result.get<BigSerial>("id", INVALID_ID);
    outObj.establishmentId = result.get<BigSerial>("establishment_id", INVALID_ID);;
    outObj.sectorId = result.get<BigSerial>("sector_id", INVALID_ID);;
    outObj.timeRestriction = result.get<int>("time_restriction", INVALID_ID);;
    outObj.isStudent = result.get<int>("is_student", INVALID_ID);;
    outObj.fixedWorkplace = result.get<int>("fixed_workplace", INVALID_ID);;
}

void JobDao::toRow(Job& data, Parameters& outParams, bool update) {}
