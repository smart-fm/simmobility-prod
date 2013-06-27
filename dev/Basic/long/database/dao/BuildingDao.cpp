/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

BuildingDao::BuildingDao(DBConnection* connection)
: AbstractDao<Building>(connection, DB_TABLE_BUILDING,
DB_INSERT_BUILDING, DB_UPDATE_BUILDING, DB_DELETE_BUILDING,
DB_GETALL_BUILDING, DB_GETBYID_BUILDING) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Building, BuildingDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Building, BuildingDao::ToRow);
}

BuildingDao::~BuildingDao() {
}

void BuildingDao::FromRow(Row& result, Building& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID);
    outObj.area = result.get<double>(DB_FIELD_AREA);
    outObj.numberOfUnits = result.get<int>(DB_FIELD_NUMBER_OF_UNITS);
    outObj.numberOfResidentialUnits = result.get<int>(DB_FIELD_NUMBER_OF_RESIDENTIAL_UNITS);
    outObj.numberOfBusinessUnits = result.get<int>(DB_FIELD_NUMBER_OF_BUSINESS_UNITS);
    outObj.numberOfUnits = result.get<int>(DB_FIELD_NUMBER_OF_STORIES);
    outObj.year = result.get<int>(DB_FIELD_YEAR);
    outObj.averageIncome = result.get<double>(DB_FIELD_AVERAGE_INCOME);
    outObj.mainRace = ToRace(result.get<int>(DB_FIELD_MAIN_RACE));
    outObj.distanceToCDB = result.get<double>(DB_FIELD_DISTANCE_TO_CDB);
}

void BuildingDao::ToRow(Building& data, Parameters& outParams, bool update) {
}