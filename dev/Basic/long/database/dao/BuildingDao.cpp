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

BuildingDao::BuildingDao(DB_Connection& connection): SqlAbstractDao<Building>( connection, 	"",	"",	"",	"",
																							"SELECT * FROM " + connection.getSchema() + "fm_building",	"" ){}

BuildingDao::~BuildingDao() {}

void BuildingDao::fromRow(Row& result, Building& outObj)
{
    outObj.fmBuildingId 		= result.get<BigSerial>("fm_building_id",INVALID_ID);
    outObj.fmProjectId 		= result.get<BigSerial>("fm_project_id",INVALID_ID);
    outObj.fmParcelId 		= result.get<BigSerial>("fm_parcel_id",INVALID_ID);
    outObj.storeysAboveGround = result.get<int>("storeys_above_ground",INVALID_ID);
    outObj.storeysBelowGround = result.get<int>("storeys_below_ground",INVALID_ID);
    outObj.fromDate 			= result.get<std::tm>(		"from_date" );
    outObj.toDate 				= result.get<std::tm>(		"to_date"   );
    outObj.buildingStatus 		= result.get<int>(	"building_status", 		0);
    outObj.grossSqMRes 		= result.get<double>(		"gross_sq_m_res", 		0.0);
    outObj.grossSqMOffice 	= result.get<double>(		"gross_sq_m_office", 	0.0);
    outObj.grossSqMRetail 	= result.get<double>(		"gross_sq_m_retail", 	0.0);
    outObj.grossSqMOther	= result.get<double>(		"gross_sq_m_other", 	0.0);
    outObj.lastChangedDate = result.get<std::tm>(       "last_changed_date", std::tm());
    outObj.freehold = result.get<int>("freehold", 0);
    outObj.floorSpace =  result.get<double>(		"floor_space", 		0.0);
    outObj.buildingType =  result.get<std::string>(	"building_type", std::string());
}

void BuildingDao::toRow(Building& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getFmBuildingId());
	outParams.push_back(data.getFmProjectId());
	outParams.push_back(data.getFmParcelId());
	outParams.push_back(data.getStoreysAboveGround());
	outParams.push_back(data.getStoreysBelowGround());
	outParams.push_back(data.getFromDate());
	outParams.push_back(data.getToDate());
	outParams.push_back(data.getBuildingStatus());
	outParams.push_back(data.getGrossSqmRes());
	outParams.push_back(data.getGrossSqmOffice());
	outParams.push_back(data.getGrossSqmRetail());
	outParams.push_back(data.getGrossSqmOther());
	outParams.push_back(data.getLastChangedDate());
	outParams.push_back(data.getFreehold());
	outParams.push_back(data.getFloorSpace());
	outParams.push_back(data.getBuildingType());
}

std::vector<Building*> BuildingDao::getBuildingsByParcelId(const long long parcelId,std::string schema)
{

	const std::string DB_GETBUILDINGS_BY_PARCELID      = "SELECT * FROM " + schema + ".fm_building" + " WHERE fm_parcel_id = :v1;";
	db::Parameters params;
	params.push_back(parcelId);
	std::vector<Building*> buildingList;
	getByQueryId(DB_GETBUILDINGS_BY_PARCELID,params,buildingList);
	return buildingList;
}

void BuildingDao::insertBuilding(Building& building,std::string schema)
{

	const std::string DB_INSERT_BUILDING_OP = "INSERT INTO " + schema + ".fm_building"
	        		+ " (" + "fm_building_id" + ", " + "fm_project_id" + ", " + "fm_parcel_id" + ", " + "storeys_above_ground"+ ", " + "storeys_below_ground" + ", " + "from_date" + ", " + "to_date"
	        		+ ", " + "building_status" + ", " + "gross_sq_m_res" + ", " + "gross_sq_m_office" + ", " + "gross_sq_m_retail" + ", " + "gross_sq_m_other" + ", " + "last_changed_date"
					+ ", " + "freehold" + ", " + "floor_space" + ", " + "building_type"
	        		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15, :v16)";
	insertViaQuery(building,DB_INSERT_BUILDING_OP);

}
