/*
 * ParcelMatchDao.cpp
 *
 *  Created on: Aug 25, 2014
 *      Author: gishara
 */
#include "ParcelMatchDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

ParcelMatchDao::ParcelMatchDao(DB_Connection& connection)
: SqlAbstractDao<ParcelMatch>(connection, "","", "", "", "SELECT * FROM " + connection.getSchema()+"parcel_match", "") {}

ParcelMatchDao::~ParcelMatchDao() {}

void ParcelMatchDao::fromRow(Row& result, ParcelMatch& outObj)
{
    outObj.fmParcelId = result.get<BigSerial>( "fm_parcel_id", INVALID_ID );
    outObj.slaParcelId = result.get<std::string>("sla_parcel_id", EMPTY_STR);
    outObj.matchCode = result.get<int>( "match_code", 0 );
    outObj.matchDate = result.get<std::tm>("match_date", std::tm());
}

void ParcelMatchDao::toRow(ParcelMatch & data, Parameters& outParams, bool update) {}



