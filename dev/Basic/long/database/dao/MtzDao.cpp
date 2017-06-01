//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licenced under of the terms of the MIT licence, as described in the file:
//licence.txt (www.opensource.org\licences\MIT)

/*
 * MtzDao.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/dao/MtzDao.hpp>
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;


MtzDao::MtzDao(DB_Connection& connection): SqlAbstractDao<Mtz>(connection, "", "", "", "", "SELECT * FROM " + connection.getSchema()+"mtz", "") {}

MtzDao::~MtzDao() {}

void MtzDao::fromRow(Row& result, Mtz& outObj)
{
	outObj.id  	= result.get<BigSerial>( "id", INVALID_ID);
	outObj.planningSubzoneId  	= result.get<BigSerial>( "planning_subzone_id", INVALID_ID);
	outObj.name = result.get<std::string>(	"name", 	EMPTY_STR);

}

void MtzDao::toRow(Mtz& data, Parameters& outParams, bool update) {}
