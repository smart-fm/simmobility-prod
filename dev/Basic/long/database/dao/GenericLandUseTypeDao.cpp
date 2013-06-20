/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   GenericLandUseTypeDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "GenericLandUseTypeDao.hpp"
#include "DatabaseHelper.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

GenericLandUseTypeDao::GenericLandUseTypeDao(DBConnection* connection)
: AbstractDao<GenericLandUseType>(connection, DB_TABLE_GENERIC_LAND_USE_TYPE,
DB_INSERT_GENERIC_LAND_USE_TYPE, DB_UPDATE_GENERIC_LAND_USE_TYPE, DB_DELETE_GENERIC_LAND_USE_TYPE,
DB_GETALL_GENERIC_LAND_USE_TYPE, DB_GETBYID_GENERIC_LAND_USE_TYPE) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(GenericLandUseType, GenericLandUseTypeDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(GenericLandUseType, GenericLandUseTypeDao::ToRow);
}

GenericLandUseTypeDao::~GenericLandUseTypeDao() {
}

void GenericLandUseTypeDao::FromRow(Row& result, GenericLandUseType& outObj) {
    outObj.id = result.get<int>(DB_FIELD_GENERIC_LAND_USE_TYPE_ID);
    outObj.unitName = result.get<string>(DB_FIELD_UNIT_NAME);
    outObj.name = result.get<string>(DB_FIELD_GENERIC_LAND_USE_NAME);
}

void GenericLandUseTypeDao::ToRow(GenericLandUseType& data, Parameters& outParams, bool update) {
}