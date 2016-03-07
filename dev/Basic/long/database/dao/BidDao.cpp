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
    outObj.sellerId = result.get<BigSerial>("seller_id",INVALID_ID);
    outObj.bidderId = result.get<BigSerial>("bidder_id",INVALID_ID);
    outObj.currentUnitId = result.get<BigSerial>("current_unit_id", INVALID_ID);
    outObj.newUnitId = result.get<BigSerial>("new_unit_id", INVALID_ID);
    outObj.willingnessToPay  = result.get<double>("willingness_to_pay", 0);
    outObj.wtpErrorTerm = result.get<double>("wtp_error_term", 0);
    outObj.affordabilityAmount = result.get<double>("affordability_amount", 0);
    outObj.currentUnitPrice = result.get<double>("current_unit_price", 0);
    outObj.targetPrice = result.get<double>("target_price", 0);
    outObj.hedonicPrice = result.get<double>("hedonic_price",0);
    outObj.lagCoefficient = result.get<double>("lag_coefficient",0);
    outObj.askingPrice = result.get<double>("asking_price",0);
    outObj.bidValue  = result.get<double>("bid_value", 0);
    outObj.bidsCounter  = result.get<int>("bids_counter", 0);
    outObj.logsum  = result.get<double>("logsum", 0);
    outObj.unitFloorArea  = result.get<double>("unit_floor_area", 0);
    outObj.unitTypeId  = result.get<BigSerial>("unit_type_id", INVALID_ID);
    outObj.currentPostcode = result.get<BigSerial>("current_postcode",INVALID_ID);
    outObj.newPostcode = result.get<BigSerial>("new_postcode",INVALID_ID);
    outObj.moveInDate = result.get<std::tm>("move_in_date",std::tm());
    outObj.accepted = result.get<int>("accepted",0);

}

void BidDao::toRow(Bid& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getBidId());
	outParams.push_back(data.getSimulationDay());
	outParams.push_back(data.getSellerId());
	outParams.push_back(data.getBidderId());
	outParams.push_back(data.getCurrentUnitId());
	outParams.push_back(data.getNewUnitId());
	outParams.push_back(data.getWillingnessToPay());
	outParams.push_back(data.getWtpErrorTerm());
	outParams.push_back(data.getAffordabilityAmount());
	outParams.push_back(data.getCurrentUnitPrice());
	outParams.push_back(data.getTargetPrice());
	outParams.push_back(data.getHedonicPrice());
	outParams.push_back(data.getLagCoefficient());
	outParams.push_back(data.getAskingPrice());
	outParams.push_back(data.getBidValue());
	outParams.push_back(data.getBidsCounter());
	outParams.push_back(data.getLogsum());
	outParams.push_back(data.getUnitFloorArea());
	outParams.push_back(data.getUnitTypeId());
	outParams.push_back(data.getCurrentPostcode());
	outParams.push_back(data.getNewPostcode());
	outParams.push_back(data.getMoveInDate());
	outParams.push_back(data.getAccepted());

}


void BidDao::insertBid(Bid& bid,std::string schema)
{

	const std::string DB_INSERT_BID = "INSERT INTO " + APPLY_SCHEMA(schema, ".bids")
                		+ " (" + "bid_id" + ", " + "simulation_day" + ", " + "seller_id" + ", " + "bidder_id"+ ", " + "current_unit_id" ", " + "new_unit_id" + ", " + "willingness_to_pay"+ ", "
                		+ "wtp_error_term"+  ", " + "affordability_amount" + ", " + "current_unit_price" + ", " + "target_price" + ", " + "hedonic_price" + ", " + "lag_coefficient" +", " + "asking_price" + ", " + "bid_value"+ ", "
                		+ "bids_counter" + ", " + "logsum" + ", "+ "unit_floor_area" + ", " + "unit_type_id" ", " + "current_postcode" + ", " + "new_postcode" + ", " + "move_in_date" +", " + "accepted"
                		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8, :v9, :v10, :v11, :v12, :v13, :v14, :v15, :v16, :v17, :v18, :v19, :v20 ,:v21, :v22, :v23)";
	insertViaQuery(bid,DB_INSERT_BID);

}



