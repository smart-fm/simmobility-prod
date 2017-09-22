/*
 * PrimarySchoolDao.cpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#include <database/dao/PrimarySchoolDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PrimarySchoolDao::PrimarySchoolDao(DB_Connection& connection): SqlAbstractDao<PrimarySchool>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"primary_schools", "")
{}

PrimarySchoolDao::~PrimarySchoolDao() {}

void PrimarySchoolDao::fromRow(Row& result, PrimarySchool& outObj)
{
    outObj.schoolId 	= result.get<BigSerial>(	"school_id", INVALID_ID);
    outObj.postcode = result.get<BigSerial>(	"postcode", INVALID_ID);
    outObj.centroidX = result.get<double>(	"centroid_x", 0);
    outObj.centroidY = result.get<double>(	"centroid_y", 0);
    outObj.schoolName = result.get<std::string>("school_name", EMPTY_STR);
    outObj.giftedProgram = result.get<int>("gifted_program", false);
    outObj.sapProgram = result.get<int>("sap_program", false);
    outObj.dgp = result.get<std::string>("dgp", EMPTY_STR);
    outObj.tazId = result.get<BigSerial>(	"taz", INVALID_ID);
}

void PrimarySchoolDao::toRow(PrimarySchool& data, Parameters& outParams, bool update) {}




