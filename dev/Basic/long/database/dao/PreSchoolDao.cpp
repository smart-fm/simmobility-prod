/*
 * PreSchoolDao.cpp
 *
 *  Created on: 15 Mar 2016
 *      Author: gishara
 */

#include <database/dao/PreSchoolDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

PreSchoolDao::PreSchoolDao(DB_Connection& connection): SqlAbstractDao<PreSchool>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"pre_schools", "")
{}

PreSchoolDao::~PreSchoolDao() {}

void PreSchoolDao::fromRow(Row& result, PreSchool& outObj)
{
    outObj.preSchoolId 	= result.get<BigSerial>(	"pre_school_id", INVALID_ID);
    outObj.preSchoolName = result.get<std::string>("pre_school_name", EMPTY_STR);
    outObj.postcode = result.get<BigSerial>(	"postcode", INVALID_ID);
    outObj.preSchoolDistrict = result.get<std::string>("pre_school_district", EMPTY_STR);
    outObj.centroidX = result.get<double>(	"centroid_x", 0);
    outObj.centroidY = result.get<double>(	"centroid_y", 0);
}

void PreSchoolDao::toRow(PreSchool& data, Parameters& outParams, bool update) {}



