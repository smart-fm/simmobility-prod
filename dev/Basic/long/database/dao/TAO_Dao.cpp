/*
 * TAO_Dao.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: gishara
 */

#include "TAO_Dao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TAO_Dao::TAO_Dao(DB_Connection& connection): SqlAbstractDao<TAO>(connection, DB_TABLE_TAO, EMPTY_STR,EMPTY_STR, EMPTY_STR, DB_GETALL_TAO, EMPTY_STR) {
}

TAO_Dao::~TAO_Dao() {
}

void TAO_Dao::fromRow(Row& result, TAO& outObj)
{
	outObj.id = result.get<BigSerial>("id",0);
	outObj.quarter = result.get<std::string>("quarter", EMPTY_STR);
	outObj.condo = result.get<double>("condo", .0);
	outObj.apartment = result.get<double>("apartment",.0);
	outObj.terrace = result.get<double>("terrace", .0);
	outObj.semi = result.get<double>("semi", .0);
	outObj.detached = result.get<double>("detached", .0);
	outObj.ec = result.get<double>("ec", .0);
}

void TAO_Dao::toRow(TAO& data, Parameters& outParams, bool update) {
}


