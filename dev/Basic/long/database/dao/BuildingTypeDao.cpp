/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   IndividualDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingTypeDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

BuildingTypeDao::BuildingTypeDao(DBConnection* connection)
: AbstractDao<BuildingType>(connection, DB_TABLE_BUILDING_TYPE,
DB_INSERT_BUILDING_TYPE, DB_UPDATE_BUILDING_TYPE, DB_DELETE_BUILDING_TYPE,
DB_GETALL_BUILDING_TYPE, DB_GETBYID_BUILDING_TYPE) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(BuildingType, BuildingTypeDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(BuildingType, BuildingTypeDao::ToRow);
}

BuildingTypeDao::~BuildingTypeDao() {
}

void BuildingTypeDao::FromRow(Row& result, BuildingType& outObj) {
    outObj.id = result.get<int>(DB_FIELD_BUILDING_TYPE_ID);
    outObj.genericId = result.get<int>(DB_FIELD_GENERIC_BUILDING_TYPE_ID);
    outObj.unitName = result.get<string>(DB_FIELD_UNIT_NAME);
    outObj.typeName = result.get<string>(DB_FIELD_BUILDING_TYPE_NAME);
    outObj.residential = (result.get<int>(DB_FIELD_IS_RESIDENTIAL) == 0) ? false : true;
    outObj.description = result.get<string>(DB_FIELD_DESCRIPTION);
    outObj.genericDescription = result.get<string>(DB_FIELD_GENERIC_BUILDING_TYPE_DESCRIPTION);
}

void BuildingTypeDao::ToRow(BuildingType& data, Parameters& outParams, bool update) {
}