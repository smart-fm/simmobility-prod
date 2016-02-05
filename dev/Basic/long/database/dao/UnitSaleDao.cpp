/*
 * UnitSaleDao.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: gishara
 */

#include "UnitSaleDao.hpp"
#include "DatabaseHelper.hpp"

using namespace sim_mob::db;
using namespace sim_mob::long_term;

UnitSaleDao::UnitSaleDao(DB_Connection& connection): SqlAbstractDao<UnitSale>(connection, EMPTY_STR,EMPTY_STR, EMPTY_STR, EMPTY_STR,EMPTY_STR, EMPTY_STR) {}

UnitSaleDao::~UnitSaleDao() {}

void UnitSaleDao::fromRow(Row& result, UnitSale& outObj)
{
    outObj.unitId  = result.get<BigSerial>("unit_id",INVALID_ID);
    outObj.buyerId = result.get<BigSerial>("buyer_id",INVALID_ID);
    outObj.sellerId = result.get<BigSerial>("seller_id",INVALID_ID);
    outObj.unitPrice  = result.get<double>("unit_price", 0);
    outObj.transactionDate = result.get<std::tm>("transaction_day",std::tm());
    outObj.daysOnMarketUnit = result.get<int>("days_on_market_unit",0);
    outObj.daysOnMarketBidder = result.get<int>("days_on_market_bidder ",0);

}

void UnitSaleDao::toRow(UnitSale& data, Parameters& outParams, bool update)
{
	outParams.push_back(data.getUnitId());
	outParams.push_back(data.getBuyerId());
	outParams.push_back(data.getSellerId());
	outParams.push_back(data.getUnitPrice());
	outParams.push_back(data.getTransactionDate());
	outParams.push_back(data.getDaysOnMarketUnit());
	outParams.push_back(data.getDaysOnMarketBidder());
}


void UnitSaleDao::insertUnitSale(UnitSale& unitSale,std::string schema)
{

	const std::string DB_INSERT_UNIT_SALE = "INSERT INTO " + APPLY_SCHEMA(schema, ".unit_sale")
                		+ " (" + "unit_id " + ", " + "buyer_id" + ", " + "seller_id" + ", " + "unit_price" ", " + "transaction_day" ", " + "days_on_market_unit" ", " + "days_on_market_bidder"
                		+ ") VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7)";
	insertViaQuery(unitSale,DB_INSERT_UNIT_SALE);

}



