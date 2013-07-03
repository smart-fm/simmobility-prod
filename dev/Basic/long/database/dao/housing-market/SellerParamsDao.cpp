/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "SellerParamsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

SellerParamsDao::SellerParamsDao(DBConnection* connection)
: AbstractDao<SellerParams>(connection, DB_TABLE_SELLER_PARAMS,
DB_INSERT_SELLER_PARAMS, DB_UPDATE_SELLER_PARAMS, DB_DELETE_SELLER_PARAMS,
DB_GETALL_SELLER_PARAMS, DB_GETBYID_SELLER_PARAMS) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(SellerParams, SellerParamsDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(SellerParams, SellerParamsDao::ToRow);
}

SellerParamsDao::~SellerParamsDao() {
}

void SellerParamsDao::FromRow(Row& result, SellerParams& outObj) {
    outObj.householdId = result.get<BigSerial>(DB_FIELD_HOUSEHOLD_ID, INVALID_ID);
    outObj.priceImportance = result.get<double>(DB_FIELD_WEIGHT_EXPECTED_EVENTS, 0);
    outObj.expectedEvents = result.get<double>(DB_FIELD_WEIGHT_PRICE_IMPORTANCE, 0);
}

void SellerParamsDao::ToRow(SellerParams& data, Parameters& outParams, bool update) {
}