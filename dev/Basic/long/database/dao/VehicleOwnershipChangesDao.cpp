/*
 * VehicleOwnershipChangesDao.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: gishara
 */
#include "VehicleOwnershipChangesDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

VehicleOwnershipChangesDao::VehicleOwnershipChangesDao(DB_Connection& connection): SqlAbstractDao<VehicleOwnershipChanges>( connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR) {}

VehicleOwnershipChangesDao::~VehicleOwnershipChangesDao() {}

void VehicleOwnershipChangesDao::fromRow(Row& result, VehicleOwnershipChanges& outObj)
{
    outObj.householdId		= result.get<BigSerial>("household_id",INVALID_ID);
    outObj.oldVehicleOwnershipOptionId		= result.get<int>("old_vehicle_ownership_option_id",0);
    outObj.newVehicleOwnershipOptionId		= result.get<int>("new_vehicle_ownership_option_id",0);
    outObj.liveInTp = result.get<int>("live_in_tp",0);
    outObj.workInTp = result.get<int>("work_in_tp",0);
   // outObj.startDate = result.get<std::tm>("start_date",std::tm());
    outObj.randomNum = result.get<double>("random_num",0);

}

void VehicleOwnershipChangesDao::toRow(VehicleOwnershipChanges& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getHouseholdId());
	outParams.push_back(data.getOldVehicleOwnershipOptionId());
	outParams.push_back(data.getNewVehicleOwnershipOptionId());
	outParams.push_back(data.isLiveInTp());
	outParams.push_back(data.isWorkInTp());
	//outParams.push_back(data.getStartDate());
	outParams.push_back(data.getRandomNum());
}

void VehicleOwnershipChangesDao::insertVehicleOwnershipChanges(VehicleOwnershipChanges& vehicleOwnershipChange,std::string schema)
{

	const std::string DB_INSERT_VEHICLE_OWNERSHIP_CHANGES = "INSERT INTO " + schema + ".vehicle_ownership_changes"
	        		+ " (" + "household_id" + ", " + "old_vehicle_ownership_option_id" + ", " + "new_vehicle_ownership_option_id" + ", "+ "live_in_tp" + ", "+ "work_in_tp" + ", " + "random_num"
	        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6)";
	insertViaQuery(vehicleOwnershipChange,DB_INSERT_VEHICLE_OWNERSHIP_CHANGES);

}








