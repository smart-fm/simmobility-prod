/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BidderParamsDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

BidderParamsDao::BidderParamsDao(DBConnection* connection)
: AbstractDao<BidderParams>(connection, DB_TABLE_BIDDER_PARAMS,
DB_INSERT_BIDDER_PARAMS, DB_UPDATE_BIDDER_PARAMS, DB_DELETE_BIDDER_PARAMS,
DB_GETALL_BIDDER_PARAMS, DB_GETBYID_BIDDER_PARAMS) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(BidderParams, BidderParamsDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(BidderParams, BidderParamsDao::ToRow);
}

BidderParamsDao::~BidderParamsDao() {
}

void BidderParamsDao::FromRow(Row& result, BidderParams& outObj) {
    outObj.householdId = result.get<BigSerial>(DB_FIELD_HOUSEHOLD_ID, INVALID_ID);
    outObj.unitAreaWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_AREA, 0);
    outObj.unitRentWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_RENT, 0);
    outObj.unitStoreyWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_STOREY, 0);
    outObj.unitTypeWeight = result.get<double>(DB_FIELD_WEIGHT_UNIT_TYPE, 0);
    outObj.urgencyToBuy = result.get<double>(DB_FIELD_WEIGHT_URGENCY_TO_BUY, 0);
}

void BidderParamsDao::ToRow(BidderParams& data, Parameters& outParams, bool update) {
}