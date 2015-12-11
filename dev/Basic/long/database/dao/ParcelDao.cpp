//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   ParcelDao.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *       : Gishara Premarathne <gishara@smart.mit.edu>
 * 
 * Created on Mar 10, 2014, 5:17 PM
 */

#include "ParcelDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ParcelDao::ParcelDao(DB_Connection& connection)
: SqlAbstractDao<Parcel>(connection, DB_TABLE_PARCEL,DB_INSERT_PARCEL, EMPTY_STR, EMPTY_STR,DB_GETALL_PARCELS, DB_GETBYID_PARCEL) {}

ParcelDao::~ParcelDao() {}

void ParcelDao::fromRow(Row& result, Parcel& outObj)
{
    outObj.id = result.get<BigSerial>( "fm_parcel_id", INVALID_ID );
    outObj.tazId = result.get<BigSerial>( "taz_id", INVALID_ID );
    outObj.lot_size = result.get<double>( "lot_size", .0 );
    outObj.gpr = result.get<std::string>("gpr", EMPTY_STR );
    outObj.land_use_type_id = result.get<int>( "land_use_type_id", 0 );
    outObj.owner_name = result.get<std::string>( "owner_name", EMPTY_STR );
    outObj.owner_category = result.get<int>( "owner_category", 0 );
    outObj.last_transaction_date = result.get<std::tm>( "last_transaction_date", std::tm() );
    outObj.last_transaction_type_total = result.get<double>( "last_transaction_type_total", .0 );
    outObj.psm_per_gps = result.get<double>( "psm_per_gps", .0);
    outObj.lease_type = result.get<int>( "lease_type", 0 );
    outObj.lease_start_date = result.get<std::tm>( "lease_start_date", std::tm() );
    outObj.centroid_x = result.get<double>( "centroid_x", .0);
    outObj.centroid_y = result.get<double>( "centroid_y", .0);
    outObj.award_date = result.get<std::tm>( "award_date", std::tm() );
    outObj.award_status = result.get<int>( "award_status", false );
    outObj.use_restriction = result.get<int>( "use_restriction", 0 );
    outObj.development_type_code = result.get<int>( "development_type_code", 0 );
    outObj.successful_tender_id = result.get<int>( "successful_tender_id", 0 );
    outObj.successful_tender_price = result.get<double>( "successful_tender_price", INVALID_ID);
    outObj.tender_closing_date = result.get<std::tm>( "tender_closing_date", std::tm() );
    outObj.lease = result.get<int>( "lease", 0 );
    outObj.status = result.get<int>("development_status",0);
    outObj.developmentAllowed = result.get<int>("development_allowed",0);
    outObj.nextAvailableDate = result.get<std::tm>("next_available_date", std::tm());
    outObj.lastChangedDate = result.get<std::tm>("last_changed_date", std::tm());
}

void ParcelDao::toRow(Parcel& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getId());
	outParams.push_back(data.getTazId());
	outParams.push_back(data.getLotSize());
	outParams.push_back(data.getGpr());
	outParams.push_back(data.getLandUseTypeId());
	outParams.push_back(data.getOwnerName());
	outParams.push_back(data.getOwnerCategory());
	outParams.push_back(data.getLastTransactionDate());
	outParams.push_back(data.getLastTransationTypeTotal());
	outParams.push_back(data.getPsmPerGps());
	outParams.push_back(data.getLeaseType());
	outParams.push_back(data.getLeaseStartDate());
	outParams.push_back(data.getCentroidX());
	outParams.push_back(data.getCentroidY());
	outParams.push_back(data.getAwardDate());
	outParams.push_back(data.getAwardStatus());
	outParams.push_back(data.getUseRestriction());
	outParams.push_back(data.getDevelopmentTypeCode());
	outParams.push_back(data.getSuccessfulTenderId());
	outParams.push_back(data.getSuccessfulTenderPrice());
	outParams.push_back(data.getTenderClosingDate());
	outParams.push_back(data.getLease());
	outParams.push_back(data.getStatus());
	outParams.push_back(data.getDevelopmentAllowed());
	outParams.push_back(data.getNextAvailableDate());
	outParams.push_back(data.getLastChangedDate());
}

std::vector<Parcel*>  ParcelDao::getEmptyParcels()
{
	const std::string queryStr = DB_GETALL_EMPTY_PARCELS;
	std::vector<Parcel*> emptyParcelList;
	getByQuery(queryStr,emptyParcelList);
	return emptyParcelList;
}

std::vector<Parcel*> ParcelDao::getParcelsWithOngoingProjects()
{
	const std::string queryStr = DB_GETALL_PARCELS_WITH_ONGOING_PROJECTS;
	std::vector<Parcel*> parcelsWithOngoingProjectsList;
	getByQuery(queryStr,parcelsWithOngoingProjectsList);
	return parcelsWithOngoingProjectsList;
}

void ParcelDao::insertParcel(Parcel& parcel,std::string schema)
{

	const std::string DB_INSERT_PARCEL_OP = "INSERT INTO " + APPLY_SCHEMA(schema, ".fm_parcel")
										+ " (" + "fm_parcel_id" + ", " + "taz_id" + ", " + "lot_size"
				                		+ ", " + "gpr" + ", " + "land_use_type_id" + ", "
				                		+ "owner_name" + ", " + "owner_category"  + ", "+ "last_transaction_date" + ", " + "last_transaction_type_total" + ", "
				                		+ "psm_per_gps" + ", " + "lease_type" + ", " + "lease_start_date"  + ", " + "centroid_x"
				                		+ ", "+ "centroid_y"  + ", " + "award_date" + ", " + "award_status" + ", " + "use_restriction" + ", " + "development_type_code"  + ", " + "successful_tender_id"
				                		+ ", "+ "successful_tender_price"  + ", " + "tender_closing_date" + ", " + "lease" + ", " + "development_status" + ", " + "development_allowed" + ", " + "next_available_date"  + ", " + "last_changed_date"
				                		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15, :v16, :v17, :v18, :v19, :v20, :v21, :v22, :v23, :v24, :v25, :v26)";
	insertViaQuery(parcel,DB_INSERT_PARCEL_OP);

}
