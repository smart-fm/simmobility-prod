//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * MtzTazDao.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/MtzTazDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;


MtzTazDao::MtzTazDao(DB_Connection& connection): SqlAbstractDao<MtzTaz>(connection, DB_TABLE_MTZ_TAZ, "", "", "", DB_GETALL_MTZ_TAZ, DB_GETBYID_MTZ_TAZ) {}

MtzTazDao::~MtzTazDao() {}

void MtzTazDao::fromRow(Row& result, MtzTaz& outObj)
{
	outObj.mtzId  	= result.get<BigSerial>( "mtz_id", INVALID_ID);
	outObj.tazId  	= result.get<BigSerial>( "taz_id", INVALID_ID);
}

void MtzTazDao::toRow(MtzTaz& data, Parameters& outParams, bool update) {}
