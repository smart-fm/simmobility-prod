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
: SqlAbstractDao<Parcel>(connection, DB_TABLE_PARCEL,EMPTY_STR, EMPTY_STR, EMPTY_STR,DB_GETALL_PARCELS, DB_GETBYID_PARCEL) {}

ParcelDao::~ParcelDao() {}

void ParcelDao::fromRow(Row& result, Parcel& outObj)
{
    outObj.id = result.get<BigSerial>( "fm_parcel_id", INVALID_ID );
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
    outObj.use_restriction = result.get<std::string>( "use_restriction", EMPTY_STR );
    outObj.development_type_code = result.get<int>( "development_type_code", 0 );
    outObj.successful_tender_id = result.get<int>( "successful_tender_id", 0 );
    outObj.successful_tender_price = result.get<double>( "successful_tender_price", INVALID_ID);
    outObj.tender_closing_date = result.get<std::tm>( "tender_closing_date", std::tm() );
    outObj.lease = result.get<int>( "lease", 0 );
    outObj.status = result.get<int>("status",0);
    outObj.developmentAllowed = result.get<std::string>("development_allowed",EMPTY_STR);
    outObj.nextAvailableDate = result.get<std::tm>("next_available_date", std::tm());
}

void ParcelDao::toRow(Parcel& data, Parameters& outParams, bool update) {}

std::vector<Parcel*>  ParcelDao::getEmptyParcels()
{
	const std::string queryStr = DB_GETALL_EMPTY_PARCELS;
	std::vector<Parcel*> emptyParcelList;
	getByQuery(queryStr,emptyParcelList);
	return emptyParcelList;
}
