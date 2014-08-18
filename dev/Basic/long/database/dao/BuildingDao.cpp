//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   BuildingDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 7, 2013, 3:59 PM
 */

#include "BuildingDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

BuildingDao::BuildingDao(DB_Connection& connection): SqlAbstractDao<Building>(connection, DB_TABLE_BUILDING,DB_INSERT_BUILDING, DB_UPDATE_BUILDING, DB_DELETE_BUILDING, DB_GETALL_BUILDING, DB_GETBYID_BUILDING) {}

BuildingDao::~BuildingDao() {
}

void BuildingDao::fromRow(Row& result, Building& outObj) {

	/*
    outObj.id = result.get<BigSerial>(DB_FIELD_ID, INVALID_ID);
    outObj.typeId = result.get<BigSerial>(DB_FIELD_TYPE_ID, INVALID_ID);
    outObj.parcelId = result.get<BigSerial>(DB_FIELD_PARCEL_ID, INVALID_ID);
    outObj.tenureId = result.get<BigSerial>(DB_FIELD_TENURE_ID, INVALID_ID);
    outObj.builtYear = result.get<int>(DB_FIELD_BUILT_YEAR, 0);
    outObj.storeys = result.get<int>(DB_FIELD_STOREYS, 0);
    outObj.parkingSpaces = result.get<int>(DB_FIELD_PARKING_SPACES, 0);
    outObj.landedArea = result.get<double>(DB_FIELD_LANDED_AREA, 0);
    */

    outObj.fm_building_id 		= result.get<BigSerial>(	"fm_building_id", 		INVALID_ID);
    outObj.fm_project_id 		= result.get<BigSerial>(	"fm_project_id", 		INVALID_ID);
    outObj.fm_parcel_id 		= result.get<BigSerial>(	"fm_parcel_id", 		INVALID_ID);
    outObj.storeys_above_ground = result.get<int>(			"storeys_above_ground", INVALID_ID);
    outObj.storeys_below_ground = result.get<int>(			"storeys_below_ground", 0);
    outObj.from_date 			= result.get<std::tm>(		"from_date" );
    outObj.to_date 				= result.get<std::tm>(		"to_date"   );
    outObj.building_status 		= result.get<std::string>(	"building_status", 		"");
    outObj.gross_sq_m_res 		= result.get<double>(		"gross_sq_m_res", 		0.0);
    outObj.gross_sq_m_office 	= result.get<double>(		"gross_sq_m_office", 	0.0);
    outObj.gross_sq_m_retail 	= result.get<double>(		"gross_sq_m_retail", 	0.0);
    outObj.gross_sq_m_other 	= result.get<double>(		"gross_sq_m_other", 	0.0);


}

void BuildingDao::toRow(Building& data, Parameters& outParams, bool update) {
}
