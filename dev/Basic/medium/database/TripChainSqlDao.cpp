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
: SqlAbstractDao<TripChainItemParams>(connection, DB_TABLE_PREDAY_FLAT_TRIPCHAINS,
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
	outParams.push_back(data.getPersonId());
	outParams.push_back(data.getTcSeqNum());
	outParams.push_back(data.getTcItemType());
	outParams.push_back(data.getTripId());
	outParams.push_back(data.getTripOrigin());
	outParams.push_back(data.getTripDestination());
	outParams.push_back(data.getSubtripId());
	outParams.push_back(data.getSubtripOrigin());
	outParams.push_back(data.getSubtripDestination());
	outParams.push_back(data.getSubtripMode());
	outParams.push_back(data.isPrimaryMode());
	outParams.push_back(data.getStartTime());
	outParams.push_back(data.getActivityId());
	outParams.push_back(data.getActivityType());
	outParams.push_back(data.isPrimaryActivity());
	outParams.push_back(data.getActivityLocation());
	outParams.push_back(data.getActivityStartTime());
	outParams.push_back(data.getActivityEndTime());
}
