/*
 * TravelTimeDao.cpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#include "TravelTimeDao.hpp"
#include "DatabaseHelper.hpp"
#include "database/entity/TravelTime.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

TravelTimeDao::TravelTimeDao(DB_Connection& connection): SqlAbstractDao<TravelTime>(connection, "", "", "", "", "SELECT * FROM " + connection.getSchema()+"travel_time", "")
{}

TravelTimeDao::~TravelTimeDao() {}

void TravelTimeDao::fromRow(Row& result, TravelTime& outObj)
{
    outObj.origin= result.get<BigSerial>("origin", INVALID_ID);
    outObj.destination = result.get<BigSerial>("destination", INVALID_ID);
    outObj.carTravelTime = result.get<double>("car_travel_time", 0);
    outObj.publicTravelTime = result.get<double>("public_travel_time", 0);

}

void TravelTimeDao::toRow(TravelTime& data, Parameters& outParams, bool update) {}

const TravelTime* TravelTimeDao::getTravelTimeByOriginDest(BigSerial origin, BigSerial destination)
{
	db::Parameters params;
	params.push_back(origin);
	params.push_back(destination);
	const std::string getTravelTime = "SELECT * FROM " + connection.getSchema() + "travel_time WHERE origin = :v1" + " AND destination = :v2;" ;
	std::vector<TravelTime*> travelTimeList;
	getByQueryId(getTravelTime,params,travelTimeList);
	TravelTime *travelTime;
	if(travelTimeList.empty())
	{
		travelTime = nullptr;
	}
	else
	{
		travelTime = travelTimeList.at(0);
	}
	return travelTime;
}

