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
#include <limits.h>

using namespace sim_mob::db;
using namespace sim_mob::long_term;

HouseholdDao::HouseholdDao(DB_Connection& connection): SqlAbstractDao<Household>(connection, "","", "", "","SELECT * FROM " + connection.getSchema()+"household", "")
{}

HouseholdDao::~HouseholdDao() {}

void HouseholdDao::fromRow(Row& result, Household& outObj)
{
    outObj.id = result.get<BigSerial>("hh_id", INVALID_ID);
    outObj.lifestyleId = result.get<BigSerial>(DB_FIELD_LIFESTYLE_ID, INVALID_ID);
    outObj.unitId = result.get<BigSerial>(DB_FIELD_UNIT_ID, INVALID_ID);
    outObj.ethnicityId = result.get<BigSerial>(DB_FIELD_ETHNICITY_ID, INVALID_ID);
    outObj.vehicleCategoryId = result.get<BigSerial>(DB_FIELD_VEHICLE_CATEGORY_ID, INVALID_ID);
    outObj.size = result.get<int>(DB_FIELD_SIZE, 0);
    outObj.childUnder4  = result.get<int>(DB_FIELD_CHILDUNDER4, 0);
    outObj.childUnder15 = result.get<int>(DB_FIELD_CHILDUNDER15, 0);
    outObj.adult = result.get<int>("num_adults", 0);
    outObj.income = std::numeric_limits<double>::min();//result.get<double>(DB_FIELD_INCOME, 0);
    outObj.housingDuration = result.get<int>(DB_FIELD_HOUSING_DURATION, 0);
    outObj.workers = result.get<int>(DB_FIELD_WORKERS, 0);
    outObj.ageOfHead = result.get<int>(DB_FIELD_AGE_OF_HEAD, 0);
    outObj.pendingStatusId = result.get<int>("pending_status_id", 0);
    outObj.pendingFromDate = result.get<std::tm>("pending_from_date", std::tm());
    outObj.unitPending = result.get<int>("unit_pending", 0);
    outObj.taxiAvailability = result.get<int>(DB_FIELD_TAXI_AVAILABILITY, false);
    outObj.vehicleOwnershipOptionId = result.get<int>("vehicle_ownership_option_id", false);
    outObj.timeOnMarket = result.get<int>("time_on_market", INVALID_ID);
    outObj.isBidder = result.get<int>("is_bidder", 0);
    outObj.isSeller = result.get<int>("is_seller", 0);
    outObj.buySellInterval = result.get<int>("buy_sell_interval", 0);
    outObj.tenureStatus = result.get<int>("tenure_status", 0);
    outObj.awakenedDay= result.get<int>("awakened_day", 0);
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
	outParams.push_back(data.getTenureStatus());
	outParams.push_back(data.getAwaknedDay());
}

void HouseholdDao::insertHousehold(Household& houseHold,std::string schema)
{

	if(houseHold.getExistInDB())
	{

		db::Parameters params;
		params.push_back(houseHold.getLifestyleId());
		params.push_back(houseHold.getUnitId());
		params.push_back(houseHold.getEthnicityId());
		params.push_back(houseHold.getVehicleCategoryId());
		params.push_back(houseHold.getSize());
		params.push_back(houseHold.getChildUnder4());
		params.push_back(houseHold.getChildUnder15());
		params.push_back(houseHold.getAdult());
		params.push_back(houseHold.getIncome());
		params.push_back(houseHold.getHousingDuration());
		params.push_back(houseHold.getWorkers());
		params.push_back(houseHold.getAgeOfHead());
		params.push_back(houseHold.getPendingStatusId());
		params.push_back(houseHold.getPendingFromDate());
		params.push_back(houseHold.getUnitPending());
		params.push_back(houseHold.getTaxiAvailability());
		params.push_back(houseHold.getVehicleOwnershipOptionId());
		params.push_back(houseHold.getTimeOnMarket());
		params.push_back(houseHold.getTimeOffMarket());
		params.push_back(houseHold.getIsBidder());
		params.push_back(houseHold.getIsSeller());
		params.push_back(houseHold.getBuySellInterval());
		params.push_back(houseHold.getTenureStatus());
		params.push_back(houseHold.getAwaknedDay());
		params.push_back(houseHold.getId());

		const std::string DB_UPDATE_HOUSEHOLD = "UPDATE "	+ schema + ".household" + " SET "
				+ DB_FIELD_LIFESTYLE_ID+ "= :v1, "
				+ DB_FIELD_UNIT_ID + "= :v2, "
				+ DB_FIELD_ETHNICITY_ID + "= :v3, "
				+ DB_FIELD_VEHICLE_CATEGORY_ID + "= :v4, "
				+ DB_FIELD_SIZE + "= :v5, "
				+ DB_FIELD_CHILDUNDER4 + "= :v6, "
				+ DB_FIELD_CHILDUNDER15 + "= :v7, "
				+ "num_adults" + "= :v8, "
				+ DB_FIELD_INCOME + "= :v9, "
				+ DB_FIELD_HOUSING_DURATION + "= :v10, "
				+ DB_FIELD_WORKERS + "= :v11, "
				+ DB_FIELD_AGE_OF_HEAD + "= :v12, "
				+ "pending_status_id" + "= :v13, "
				+ "pending_from_date" + "= :v14, "
				+ "unit_pending" + "= :v15, "
				+ DB_FIELD_TAXI_AVAILABILITY + "= :v16, "
				+ "vehicle_ownership_option_id" + "= :v17, "
				+ "time_on_market" + "= :v18, "
				+ "time_off_market"+ "= :v19, "
				+ "is_bidder" + "= :v20, "
				+ "is_seller" + "= :v21, "
				+ "buy_sell_interval" + "= :v22, "
				+ "tenure_status" + "= :v23, "
				+ "awakened_day"
				+ "= :v24 WHERE "
				+ DB_FIELD_ID + "=:v25";
		executeQueryWithParams(houseHold,DB_UPDATE_HOUSEHOLD,params);
	}

	else
	{
		const std::string DB_INSERT_HOUSEHOLD_OP = "INSERT INTO " + schema + ".household"
							+ " (" + DB_FIELD_ID + ", "+ "lifestyle_id" + ", "+ DB_FIELD_UNIT_ID + ", "+ DB_FIELD_ETHNICITY_ID + ", "+ DB_FIELD_VEHICLE_CATEGORY_ID + ", "+ DB_FIELD_SIZE + ", "+
							DB_FIELD_CHILDUNDER4 + ", "+ DB_FIELD_CHILDUNDER15 + ", " + "num_adults" + ", "+ DB_FIELD_INCOME + ", "+ DB_FIELD_HOUSING_DURATION + ", " + "workers"+ ", "+
							"age_of_head" + ", "+ "pending_status_id" + ", " + "pending_from_date" + ", "+ "unit_pending" + ", "+ "taxi_availability" + ", " + "vehicle_ownership_option_id"+ ", "+
							+ "time_on_market" + ", " + "time_off_market"+ ", "+ "is_bidder" + ", " + "is_seller"+ ", "+ "buy_sell_interval" + ", "+
							+ "tenure_status"  ", " + "awakened_day" + ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7 ,:v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15, :v16, :v17, :v18, :v19, :v20, :v21, :v22, :v23, :v24, :V25)";
		insertViaQuery(houseHold,DB_INSERT_HOUSEHOLD_OP);
	}

}
