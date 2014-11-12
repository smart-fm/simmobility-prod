/*
 * SlaParcelDao.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gishara
 */
#include "SlaParcelDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

SlaParcelDao::SlaParcelDao(DB_Connection& connection): SqlAbstractDao<SlaParcel>(connection, DB_TABLE_SLA_PARCEL, EMPTY_STR,EMPTY_STR, EMPTY_STR, DB_GETALL_SLA_PARCELS, EMPTY_STR) {
}

SlaParcelDao::~SlaParcelDao() {
}

void SlaParcelDao::fromRow(Row& result, SlaParcel& outObj) {
	outObj.slaId = result.get<BigSerial>("sla_id", INVALID_ID);
	outObj.tazId = result.get<BigSerial>("taz_id", INVALID_ID);
	outObj.landUseZoneId = result.get<BigSerial>("land_use_zone_id",INVALID_ID);
	outObj.area = result.get<double>("area", .0);
	outObj.length = result.get<double>("length", .0);
	outObj.minX = result.get<double>("min_x", .0);
	outObj.minY = result.get<double>("min_y", .0);
	outObj.maxX = result.get<double>("max_x", .0);
	outObj.maxY = result.get<double>("max_y", .0);
}

void SlaParcelDao::toRow(SlaParcel& data, Parameters& outParams, bool update) {
}

