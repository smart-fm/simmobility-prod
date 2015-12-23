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
    outObj.vehicleOwnershipOptionId		= result.get<int>("vehicle_ownership_option_id",0);
    outObj.startDate = result.get<std::tm>("start_date",std::tm());
}

void VehicleOwnershipChangesDao::toRow(VehicleOwnershipChanges& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getHouseholdId());
	outParams.push_back(data.getVehicleOwnershipOptionId());
	outParams.push_back(data.getStartDate());
}

void VehicleOwnershipChangesDao::insertVehicleOwnershipChanges(VehicleOwnershipChanges& vehicleOwnershipChange,std::string schema)
{

	const std::string DB_INSERT_VEHICLE_OWNERSHIP_CHANGES = "INSERT INTO " + APPLY_SCHEMA(schema, ".vehicle_ownership_changes")
	        		+ " (" + "household_id" + ", " + "vehicle_ownership_option_id" + ", " + "start_date"
	        		+ ") VALUES (:v1, :v2, :v3)";
	insertViaQuery(vehicleOwnershipChange,DB_INSERT_VEHICLE_OWNERSHIP_CHANGES);

}








