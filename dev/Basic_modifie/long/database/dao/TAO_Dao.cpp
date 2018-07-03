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

TAO_Dao::TAO_Dao(DB_Connection& connection): SqlAbstractDao<TAO>(connection, "", "","", "", "SELECT * FROM " + connection.getSchema()+"tao_hedonic_price", "") {
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
	outObj.hdb12 = result.get<double>("hdb12", .0);
	outObj.hdb3 = result.get<double>("hdb3", .0);
	outObj.hdb4 = result.get<double>("hdb4", .0);
	outObj.hdb5 = result.get<double>("hdb5", .0);
	outObj.exec = result.get<double>("exec", .0);
}

void TAO_Dao::toRow(TAO& data, Parameters& outParams, bool update) {
}


