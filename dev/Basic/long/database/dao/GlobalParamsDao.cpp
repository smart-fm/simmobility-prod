/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:59 PM
 */

#include "GlobalParamsDao.hpp"
#include "DatabaseHelper.hpp"
#include "database/entity/GlobalParams.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;
using std::string;

GlobalParamsDao::GlobalParamsDao(DBConnection* connection)
: AbstractDao<GlobalParams>(connection, DB_TABLE_GLOBAL_PARAMS,
DB_INSERT_GLOBAL_PARAMS, DB_UPDATE_GLOBAL_PARAMS, DB_DELETE_GLOBAL_PARAMS,
DB_GETALL_GLOBAL_PARAMS, DB_GETBYID_GLOBAL_PARAMS) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(GlobalParams, GlobalParamsDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(GlobalParams, GlobalParamsDao::ToRow);
}

GlobalParamsDao::~GlobalParamsDao() {
}

void GlobalParamsDao::FromRow(Row& result, GlobalParams& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.unitAreaWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_AREA, 0);
    outObj.unitRentWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_RENT, 0);
    outObj.unitStoreyWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_STOREY, 0);
    outObj.unitTypeWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_TYPE, 0);
}

void GlobalParamsDao::ToRow(GlobalParams& data, Parameters& outParams, bool update) {
}