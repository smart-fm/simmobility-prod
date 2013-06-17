/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LandUseTypeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "LandUseTypeDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

LandUseTypeDao::LandUseTypeDao(DBConnection* connection)
: AbstractDao<LandUseType>(connection, DB_TABLE_LAND_USE_TYPE,
DB_INSERT_LAND_USE_TYPE, DB_UPDATE_LAND_USE_TYPE, DB_DELETE_LAND_USE_TYPE,
DB_GETALL_LAND_USE_TYPE, DB_GETBYID_LAND_USE_TYPE) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(LandUseType, LandUseTypeDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(LandUseType, LandUseTypeDao::ToRow);
}

LandUseTypeDao::~LandUseTypeDao() {
}

void LandUseTypeDao::FromRow(Row& result, LandUseType& outObj) {
    outObj.id = result.get<int>(DB_FIELD_LAND_USE_TYPE_ID);
    outObj.genericTypeId = result.get<int>(DB_FIELD_GENERIC_LAND_USE_TYPE_ID);
    outObj.unitName = result.get<string>(DB_FIELD_UNIT_NAME);
    outObj.name = result.get<string>(DB_FIELD_LAND_USE_NAME);
    outObj.description = result.get<string>(DB_FIELD_DESCRIPTION);
}

void LandUseTypeDao::ToRow(LandUseType& data, Parameters& outParams, bool update) {
}