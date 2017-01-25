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
    outObj.buildingId 	= result.get<BigSerial>("fm_building_id", INVALID_ID);
    outObj.firmId 		= result.get<BigSerial>("firm_id", INVALID_ID);
    outObj.firmFoundationYear 	= result.get<BigSerial>("firm_foundation_year",0);
    outObj.industryTypeId 	= result.get<int>("industry_type_id",0);
    outObj.floorArea 		= result.get<double>("floor_area",0);
    outObj.jobSize 			= result.get<int>("job_size",0);
    outObj.revenue 			= result.get<double>("revenue",0);
    outObj.capital 			= result.get<double>("capital",0);
    outObj.establishmentLifestyleId = result.get<BigSerial>("establishment_lifestyle_id", 0);
}

void EstablishmentDao::toRow(Establishment& data, Parameters& outParams, bool update) {}

