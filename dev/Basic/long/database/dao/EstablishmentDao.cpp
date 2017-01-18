/*
 * EstablishmentDao.cpp
 *
 *  Created on: 23 Apr, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/EstablishmentDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

EstablishmentDao::EstablishmentDao(DB_Connection& connection): SqlAbstractDao<Establishment>(connection, DB_TABLE_ESTABLISHMENT,EMPTY_STR,
																							 EMPTY_STR, EMPTY_STR,DB_GETALL_ESTABLISHMENT, DB_GETBYID_ESTABLISHMENT){}

EstablishmentDao::~EstablishmentDao() {}

void EstablishmentDao::fromRow(Row& result, Establishment& outObj)
{
    outObj.id 			= result.get<BigSerial>("id", INVALID_ID);
    outObj.firmId 		= result.get<BigSerial>("firm_id", INVALID_ID);
    outObj.buildingId 	= result.get<BigSerial>("fm_building_id", INVALID_ID);
    outObj.revenue 		= result.get<double>("revenue", 0);
    outObj.lifestyleId 	= result.get<BigSerial>("establishment_lifestyle_id", 0);
    //outObj.businessTypeId = result.get<BigSerial>("business_type_id", 0);
    //outObj.size 		= result.get<int>("size", 0);
    //outObj.slaAddressId = result.get<BigSerial>("sla_address_id", 0);
    //outObj.grossSqM 	= result.get<double>("gross_sq_m", 0);
    //outObj.sectorId 	= result.get<int>("sector_id",0);

}

void EstablishmentDao::toRow(Establishment& data, Parameters& outParams, bool update) {}

