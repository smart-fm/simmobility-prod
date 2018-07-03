/*
 * UnitSale.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: gishara
 */

#include "UnitSale.hpp"

using namespace sim_mob::long_term;

UnitSale::UnitSale(BigSerial unitSaleId,BigSerial unitId,BigSerial buyerId, BigSerial sellerId, double unitPrice,std::tm transactionDate,int daysOnMarketUnit, int daysOnMarketBidder):
		unitSaleId(unitSaleId),unitId(unitId),buyerId(buyerId),sellerId(sellerId),unitPrice(unitPrice),transactionDate(transactionDate),daysOnMarketUnit(daysOnMarketUnit),daysOnMarketBidder(daysOnMarketBidder){}


UnitSale::UnitSale(const UnitSale& source)
{
	this->unitSaleId = source.unitSaleId;
	this->unitId = source.unitId;
	this->buyerId = source.buyerId;
	this->sellerId = source.sellerId;
	this->unitPrice = source.unitPrice;
	this->transactionDate = source.transactionDate;
	this->daysOnMarketBidder = source.daysOnMarketBidder;
	this->daysOnMarketUnit = source.daysOnMarketUnit;
}

UnitSale& UnitSale::operator=(const UnitSale& source)
{
	this->unitSaleId = source.unitSaleId;
	this->unitId = source.unitId;
	this->buyerId = source.buyerId;
	this->sellerId = source.sellerId;
	this->unitPrice = source.unitPrice;
	this->transactionDate = source.transactionDate;
	this->daysOnMarketBidder = source.daysOnMarketBidder;
	this->daysOnMarketUnit = source.daysOnMarketUnit;
    return *this;
}

UnitSale::~UnitSale() {}

BigSerial UnitSale::getBuyerId() const
{
	return buyerId;
}

void UnitSale::setBuyerId(BigSerial buyerId)
{
	this->buyerId = buyerId;
}

BigSerial UnitSale::getSellerId() const
{
	return sellerId;
}

void UnitSale::setSellerId(BigSerial sellerId)
{
	this->sellerId = sellerId;
}

const std::tm& UnitSale::getTransactionDate() const
{
	return transactionDate;
}

void UnitSale::setTransactionDate(const std::tm& transactionDate)
{
	this->transactionDate = transactionDate;
}

BigSerial UnitSale::getUnitSaleId() const
{
	return unitSaleId;
}

void UnitSale::setUnitSaleId(BigSerial unitSaleId)
{
		this->unitSaleId = unitSaleId;
}


BigSerial UnitSale::getUnitId() const
{
	return unitId;
}

void UnitSale::setUnitId(BigSerial unitId)
{
		this->unitId = unitId;
}

double UnitSale::getUnitPrice() const
{
		return unitPrice;
}

void UnitSale::setUnitPrice(double unitPrice)
{
	this->unitPrice = unitPrice;
}

int UnitSale::getDaysOnMarketBidder() const
{
	return daysOnMarketBidder;
}

void UnitSale::setDaysOnMarketBidder(int daysOnMarketBidder)
{
	this->daysOnMarketBidder = daysOnMarketBidder;
}

int UnitSale::getDaysOnMarketUnit() const
{
		return daysOnMarketUnit;
}

void UnitSale::setDaysOnMarketUnit(int daysOnMarketUnit)
{
		this->daysOnMarketUnit = daysOnMarketUnit;
}
