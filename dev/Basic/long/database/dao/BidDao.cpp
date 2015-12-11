/*
 * BidDao.cpp
 *
 *  Created on: Nov 30, 2015
 *      Author: gishara
 */

#include "BidDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

BidDao::BidDao(DB_Connection& connection): SqlAbstractDao<Bid>(connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR) {}

BidDao::~BidDao() {}

void BidDao::fromRow(Row& result, Bid& outObj)
{
    outObj.bidId  = result.get<BigSerial>("bid_id",INVALID_ID);
    outObj.simulationDay = result.get<int>("simulation_day",0);
    outObj.bidderId = result.get<BigSerial>("bidder_id",INVALID_ID);
    outObj.currentUnitId = result.get<BigSerial>("current_unit_id", INVALID_ID);
    outObj.newUnitId = result.get<BigSerial>("new_unit_id", INVALID_ID);
    outObj.willingnessToPay  = result.get<double>("willingness_to_pay", 0);
    outObj.affordabilityAmount = result.get<double>("affordability_amount", 0);
    outObj.hedonicPrice = result.get<double>("hedonic_price",0);
    outObj.askingPrice = result.get<double>("asking_price",0);
    outObj.targetPrice = result.get<double>("target_price",0);
    outObj.bidValue  = result.get<double>("bid_value", 0);
    outObj.isAccepted = result.get<int>("is_accpeted",0);
    outObj.currentPostcode = result.get<BigSerial>("current_postcode",INVALID_ID);
    outObj.newPostcode = result.get<BigSerial>("new_postcode",INVALID_ID);

}

void BidDao::toRow(Bid& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getBidId());
	outParams.push_back(data.getSimulationDay());
	outParams.push_back(data.getBidderId());
	outParams.push_back(data.getCurrentUnitId());
	outParams.push_back(data.getNewUnitId());
	outParams.push_back(data.getWillingnessToPay());
	outParams.push_back(data.getAffordabilityAmount());
	outParams.push_back(data.getHedonicPrice());
	outParams.push_back(data.getAskingPrice());
	outParams.push_back(data.getTargetPrice());
	outParams.push_back(data.getBidValue());
	outParams.push_back(data.getIsAccepted());
	outParams.push_back(data.getCurrentPostcode());
	outParams.push_back(data.getNewPostcode());

}


void BidDao::insertBid(Bid& bid,std::string schema)
{

	const std::string DB_INSERT_BID = "INSERT INTO " + APPLY_SCHEMA(schema, "bids")
                		+ " (" + "bid_id" + ", " + "simulation_day" + ", " + "bidder_id" + ", " + "current_unit_id" ", " + "new_unit_id" + ", " + "willingness_to_pay"
                		+ "affordability_amount" + ", " + "hedonic_price" + ", " + "asking_price" + ", " + "target_price" + ", " + "bid_value"
                		+ "is_accpeted" + ", " + "current_postcode" + ", " + "new_postcode"
                		+ ") VALUES (:v1, :v2, :v3, :v4, :v5,:v6, :v7, :v8, :v9, :v10, :v11, :v12, :v13, :v14)";
	insertViaQuery(bid,DB_INSERT_BID);

}



