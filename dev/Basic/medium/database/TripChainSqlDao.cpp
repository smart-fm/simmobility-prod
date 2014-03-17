//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * TripChainSqlDao.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: harish
 */

#include "TripChainSqlDao.hpp"
#include "DatabaseHelper.hpp"

sim_mob::medium::TripChainSqlDao::TripChainSqlDao(db::DB_Connection& connection)
: SqlAbstractDao<TripChainItemParams*>(connection, DB_TABLE_PREDAY_FLAT_TRIPCHAINS,
		DB_INSERT_TRIP_CHAIN_ITEM, "", "", "", "")
{}

sim_mob::medium::TripChainSqlDao::~TripChainSqlDao()
{}

void sim_mob::medium::TripChainSqlDao::fromRow(db::Row& result,
		TripChainItemParams& outObj) {
	throw std::runtime_error("Reading from trip chain table is not implemented yet.");
}

void sim_mob::medium::TripChainSqlDao::toRow(TripChainItemParams& data,
		db::Parameters& outParams, bool update) {

}
