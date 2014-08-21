//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "UnitDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

UnitDao::UnitDao(DB_Connection& connection): SqlAbstractDao<Unit>(connection, DB_TABLE_UNIT,DB_INSERT_UNIT, DB_UPDATE_UNIT, DB_DELETE_UNIT,DB_GETALL_UNIT, DB_GETBYID_UNIT) {}

UnitDao::~UnitDao() {}

void UnitDao::fromRow(Row& result, Unit& outObj)
{
    outObj.id  = result.get<BigSerial>("fm_unit_id", INVALID_ID);
    outObj.building_id  = result.get<BigSerial>("fm_building_id", INVALID_ID);
    outObj.sla_address_id  = result.get<BigSerial>("sla_address_id", INVALID_ID);
    outObj.unit_type  = result.get<int>("unit_type", INVALID_ID);
    outObj.storey_range  = result.get<int>("storey_range", 0);
    outObj.unit_status  = result.get<std::string>("unit_status", EMPTY_STR);
    outObj.floor_area  = result.get<double>("floor_area", .0);
    outObj.storey  = result.get<int>("storey", 0);
    outObj.rent  = result.get<double>("rent", .0);
    outObj.sale_from_date  = result.get<std::tm>("sale_from_date", std::tm());
    outObj.physical_from_date  = result.get<std::tm>("physical_from_date", std::tm());
    outObj.sale_status  = result.get<int>("sale_status", 0);
    outObj.physical_status  = result.get<int>("physical_status", 0);
}

void UnitDao::toRow(Unit& data, Parameters& outParams, bool update) {}
