//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 23, 2013, 5:17 PM
 */

#include "HouseholdDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HouseholdDao::HouseholdDao(DB_Connection& connection): SqlAbstractDao<Household>(connection, DB_TABLE_HOUSEHOLD,DB_INSERT_HOUSEHOLD, DB_UPDATE_HOUSEHOLD, DB_DELETE_HOUSEHOLD,DB_GETALL_HOUSEHOLD, DB_GETBYID_HOUSEHOLD)
{}

HouseholdDao::~HouseholdDao() {}

void HouseholdDao::fromRow(Row& result, Household& outObj)
{
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.lifestyleId = result.get<BigSerial>(DB_FIELD_LIFESTYLE_ID, INVALID_ID);
    outObj.unitId = result.get<BigSerial>(DB_FIELD_UNIT_ID, INVALID_ID);
    outObj.ethnicityId = result.get<BigSerial>(DB_FIELD_ETHNICITY_ID, INVALID_ID);
    outObj.vehicleCategoryId = result.get<BigSerial>(DB_FIELD_VEHICLE_CATEGORY_ID, INVALID_ID);
    outObj.size = result.get<int>(DB_FIELD_SIZE, 0);
    outObj.childUnder4  = result.get<int>(DB_FIELD_CHILDUNDER4, 0);
    outObj.childUnder15 = result.get<int>(DB_FIELD_CHILDUNDER15, 0);
    outObj.adult = result.get<int>("adult", 0);
    outObj.income = result.get<double>(DB_FIELD_INCOME, 0);
    outObj.housingDuration = result.get<int>(DB_FIELD_HOUSING_DURATION, 0);
    outObj.workers = result.get<int>(DB_FIELD_WORKERS, 0);
    outObj.ageOfHead = result.get<int>(DB_FIELD_AGE_OF_HEAD, 0);
    outObj.pendingStatusId = result.get<int>("pending_status_id", 0);
    outObj.pendingFromDate = result.get<std::tm>("pending_from_date", std::tm());
    outObj.unitPending = result.get<int>("unit_pending", 0);
    outObj.taxiAvailability = result.get<int>(DB_FIELD_TAXI_AVAILABILITY, false);
    outObj.vehicleOwnershipOptionId = result.get<int>("vehicle_ownership_option_id", false);
    outObj.timeOnMarket = result.get<int>("time_on_market", INVALID_ID);
    outObj.timeOffMarket = result.get<int>("time_off_market", INVALID_ID);
    outObj.isBidder = result.get<int>("is_bidder", 0);
    outObj.isSeller = result.get<int>("is_seller", 0);
    outObj.buySellInterval = result.get<int>("buy_sell_interval", 0);
    outObj.moveInDate = result.get<std::tm>("move_in_date", std::tm());
    outObj.hasMoved = result.get<int>("has_moved", 0);
    outObj.tenureStatus = result.get<int>("tenure_status", 0);
}

void HouseholdDao::toRow(Household& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getId());
	outParams.push_back(data.getLifestyleId());
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getEthnicityId());
	outParams.push_back(data.getVehicleCategoryId());
	outParams.push_back(data.getSize());
	outParams.push_back(data.getChildUnder4());
	outParams.push_back(data.getChildUnder15());
	outParams.push_back(data.getAdult());
	outParams.push_back(data.getIncome());
	outParams.push_back(data.getHousingDuration());
	outParams.push_back(data.getWorkers());
	outParams.push_back(data.getAgeOfHead());
	outParams.push_back(data.getPendingStatusId());
	outParams.push_back(data.getPendingFromDate());
	outParams.push_back(data.getUnitPending());
	outParams.push_back(data.getTaxiAvailability());
	outParams.push_back(data.getVehicleOwnershipOptionId());
	outParams.push_back(data.getTimeOnMarket());
	outParams.push_back(data.getTimeOffMarket());
	outParams.push_back(data.getIsBidder());
	outParams.push_back(data.getIsSeller());
	outParams.push_back(data.getBuySellInterval());
	outParams.push_back(data.getMoveInDate());
	outParams.push_back(data.getHasMoved());
	outParams.push_back(data.getTenureStatus());
}

void HouseholdDao::insertHousehold(Household& houseHold,std::string schema)
{
	const std::string DB_INSERT_HOUSEHOLD_OP = "INSERT INTO " + APPLY_SCHEMA(schema, ".household")
					+ " (" + DB_FIELD_ID + ", "+ "lifestyle_id" + ", "+ DB_FIELD_UNIT_ID + ", "+ "ethnicity_id" + ", "+ "vehicle_category_id" + ", "+ DB_FIELD_SIZE + ", "+
					DB_FIELD_CHILDUNDER4 + ", "+ DB_FIELD_CHILDUNDER15 + ", " + "adult" + ", "+ DB_FIELD_INCOME + ", "+ DB_FIELD_HOUSING_DURATION + ", " + "workers"+ ", "+
					"age_of_head" + ", "+ "pending_status_id" + ", " + "pending_from_date" + ", "+ "unit_pending" + ", "+ "taxi_availability" + ", " + "vehicle_ownership_option_id"+ ", "+
					+ "time_on_market" + ", " + "time_off_market"+ ", "+ "is_bidder" + ", " + "is_seller"+ ", "+ "buy_sell_interval" + ", "+ "move_in_date" + ", " + "has_moved"+ ", "+ "tenure_status"
					+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7 ,:v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15, :v16, :v17, :v18, :v19, :v20, :v21, :v22, :v23, :v24, :V25, :v26)";
	insertViaQuery(houseHold,DB_INSERT_HOUSEHOLD_OP);
}
