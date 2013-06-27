/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "UnitDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

UnitDao::UnitDao(DBConnection* connection)
: AbstractDao<Unit>(connection, DB_TABLE_UNIT,
DB_INSERT_UNIT, DB_UPDATE_UNIT, DB_DELETE_UNIT,
DB_GETALL_UNIT, DB_GETBYID_UNIT) {
    fromRowCallback = DAO_FROM_ROW_CALLBACK_HANDLER(Unit, UnitDao::FromRow);
    toRowCallback = DAO_TO_ROW_CALLBACK_HANDLER(Unit, UnitDao::ToRow);
}

UnitDao::~UnitDao() {
}

void UnitDao::FromRow(Row& result, Unit& outObj) {
    outObj.id = result.get<BigSerial>(DB_FIELD_ID);
    outObj.buildingId = result.get<BigSerial>(DB_FIELD_BUILDING_ID);
    outObj.householdId = result.get<BigSerial>(DB_FIELD_HOUSEHOLD_ID);
    outObj.type = ToUnitType(result.get<int>(DB_FIELD_TYPE));
    outObj.storey = result.get<int>(DB_FIELD_STOREY);
    outObj.lastRemodulationYear = result.get<int>(DB_FIELD_YEAR_OF_LAST_REMODULATION);
    outObj.area = result.get<double>(DB_FIELD_AREA);
    outObj.fixedPrice = result.get<double>(DB_FIELD_FIXED_PRICE);
    outObj.taxExempt = result.get<double>(DB_FIELD_TAX_EXEMPT);
    outObj.hedonicPrice = result.get<double>(DB_FIELD_HEDONIC_PRICE);
    outObj.distanceToCBD = result.get<double>(DB_FIELD_DISTANCE_TO_CDB);
    outObj.hasGarage = (result.get<int>(DB_FIELD_HAS_GARAGE) != 0 ? true : false);
    outObj.weightPriceQuality = result.get<double>(DB_FIELD_WEIGHT_PRICE_QUALITY);
    outObj.weightStorey = result.get<double>(DB_FIELD_WEIGHT_STOREY);
    outObj.weightDistanceToCBD = result.get<double>(DB_FIELD_WEIGHT_DISTANCE_TO_CBD);
    outObj.weightType = result.get<double>(DB_FIELD_WEIGHT_TYPE);
    outObj.weightArea = result.get<double>(DB_FIELD_WEIGHT_AREA);
    outObj.weightTaxExempt = result.get<double>(DB_FIELD_WEIGHT_TAX_EXEMPT);
    outObj.weightYearLastRemodulation = result.get<double>(DB_FIELD_WEIGHT_YEAR_LAST_REMODULATION);
    outObj.reservationPrice = 0;
    outObj.available = false;
}

void UnitDao::ToRow(Unit& data, Parameters& outParams, bool update) {
}