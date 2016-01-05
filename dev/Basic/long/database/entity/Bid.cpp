//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Bid.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 5, 2013, 5:03 PM
 */

#include "Bid.hpp"
#include "metrics/Frame.hpp"

using namespace sim_mob::long_term;

Bid::Bid(BigSerial bidId,int simulationDay, BigSerial bidderId, BigSerial currentUnitId, BigSerial newUnitId,double willingnessToPay,double affordabilityAmount,double hedonicPrice,
		double askingPrice,double targetPrice,double bidValue, int isAccepted,BigSerial currentPostcode, BigSerial newPostcode,Agent_LT* bidder,std::tm moveInDate, double wtpErrorTerm)
		:bidId(bidId),simulationDay(simulationDay),bidderId(bidderId),currentUnitId(currentUnitId),newUnitId(newUnitId), willingnessToPay(willingnessToPay),
		 affordabilityAmount(affordabilityAmount), hedonicPrice(hedonicPrice), askingPrice(askingPrice),targetPrice(targetPrice),bidValue(bidValue),
		 isAccepted(isAccepted),currentPostcode(currentPostcode),newPostcode(newPostcode),bidder(bidder),moveInDate(moveInDate), wtpErrorTerm(wtpErrorTerm){}

Bid::Bid(BigSerial bidId,BigSerial currentUnitId,BigSerial newUnitId, BigSerial bidderId,Agent_LT* bidder,double bidValue, int simulationDay, double willingnessToPay, double wtp_e, double affordabilityAmount)
		:bidId(bidId),currentUnitId(currentUnitId), newUnitId(newUnitId),bidderId(bidderId),bidder(bidder),bidValue(bidValue), simulationDay(simulationDay) , willingnessToPay(willingnessToPay)
         ,hedonicPrice(0), askingPrice(askingPrice),targetPrice(targetPrice), isAccepted(0),currentPostcode(0),newPostcode(0),moveInDate(tm()), wtpErrorTerm(wtp_e), affordabilityAmount(affordabilityAmount){}

Bid::Bid(const Bid& source)
{
	this->bidId = source.bidId;
	this->simulationDay = source.simulationDay;
	this->bidderId = source.bidderId;
	this->currentUnitId = source.currentUnitId;
    this->newUnitId = source.newUnitId;
    this->willingnessToPay = source.willingnessToPay;
    this->affordabilityAmount = source.affordabilityAmount;
    this->hedonicPrice = source.hedonicPrice;
    this->askingPrice = source.askingPrice;
    this->targetPrice = source.targetPrice;
    this->bidValue = source.bidValue;
    this->isAccepted = source.isAccepted;
    this->currentPostcode = source.currentPostcode;
    this->newPostcode = source.newPostcode;
    this->bidder = source.bidder;
    this->moveInDate = source.moveInDate;
    this->wtpErrorTerm = source.wtpErrorTerm;
}

Bid::Bid(): bidId(bidId),simulationDay(simulationDay),bidderId(INVALID_ID),currentUnitId(INVALID_ID),newUnitId(INVALID_ID),willingnessToPay(0.0),affordabilityAmount(0.0),hedonicPrice(0.0),
		askingPrice(0.0),targetPrice(0.0),bidValue(0.0),isAccepted(0),currentPostcode(INVALID_ID),newPostcode(INVALID_ID),bidder(nullptr),moveInDate(moveInDate), wtpErrorTerm(0){}

Bid::~Bid() {}

Bid& Bid::operator=(const Bid& source)
{
	this->bidId = source.bidId;
	this->simulationDay = source.simulationDay;
	this->bidderId = source.bidderId;
	this->currentUnitId = source.currentUnitId;
	this->newUnitId = source.newUnitId;
	this->willingnessToPay = source.willingnessToPay;
	this->affordabilityAmount = source.affordabilityAmount;
	this->hedonicPrice = source.hedonicPrice;
	this->askingPrice = source.askingPrice;
	this->targetPrice = source.targetPrice;
	this->bidValue = source.bidValue;
	this->isAccepted = source.isAccepted;
	this->currentPostcode = source.currentPostcode;
	this->newPostcode = source.newPostcode;
	this->bidder = source.bidder;
	this->moveInDate = source.moveInDate;
	this->wtpErrorTerm = source.wtpErrorTerm;
    return *this;
}

BigSerial Bid::getBidId() const
{
	return this->bidId;
}

int Bid::getSimulationDay() const
{
	return this->simulationDay;
}

BigSerial Bid::getBidderId() const
{
    return this->bidderId;
}

BigSerial Bid::getCurrentUnitId() const
{
    return this->currentUnitId;
}

BigSerial Bid::getNewUnitId() const
{
    return this->newUnitId;
}

double Bid::getWillingnessToPay() const
{
    return this->willingnessToPay;
}

double Bid::getAffordabilityAmount() const
{
	return this->affordabilityAmount;
}

double Bid::getHedonicPrice() const
{
	return this->hedonicPrice;
}

double Bid::getAskingPrice() const
{
	return this->askingPrice;
}

double Bid::getTargetPrice() const
{
	return this->targetPrice;
}

double Bid::getBidValue() const
{
    return this->bidValue;
}

int Bid::getIsAccepted() const
{
	return this->isAccepted;
}

BigSerial Bid::getCurrentPostcode() const
{
	return this->currentPostcode;
}

BigSerial Bid::getNewPostcode() const
{
	return this->newPostcode;
}

const std::tm& Bid::getMoveInDate() const
{
		return moveInDate;
}

void Bid::setAffordabilityAmount(double affordabilityAmount)
{
	this->affordabilityAmount = affordabilityAmount;
}

void Bid::setAskingPrice(double askingPrice) {
	this->askingPrice = askingPrice;
}

void Bid::setBidderId(BigSerial bidderId)
{
	this->bidderId = bidderId;
}

void Bid::setBidId(BigSerial bidId)
{
	this->bidId = bidId;
}

void Bid::setBidValue(double bidValue)
{
	this->bidValue = bidValue;
}

void Bid::setCurrentPostcode(BigSerial currentPostcode)
{
	this->currentPostcode = currentPostcode;
}

void Bid::setHedonicPrice(double hedonicPrice)
{
	this->hedonicPrice = hedonicPrice;
}

void Bid::setIsAccepted(int isAccepted)
{
	this->isAccepted = isAccepted;
}

void Bid::setNewPostcode(BigSerial newPostcode)
{
	this->newPostcode = newPostcode;
}

void Bid::setSimulationDay(int simulationDay)
{
	this->simulationDay = simulationDay;
}

void Bid::setTargetPrice(double targetPrice)
{
	this->targetPrice = targetPrice;
}

void Bid::setCurrentUnitId(BigSerial currUnitId)
{
	this->currentUnitId = currUnitId;
}

void Bid::setNewUnitId(BigSerial unitId)
{
	this->newUnitId = unitId;
}

void Bid::setWillingnessToPay(double willingnessToPay)
{
	this->willingnessToPay = willingnessToPay;
}

void Bid::setMoveInDate(const std::tm& moveInDate)
{
		this->moveInDate = moveInDate;
}

Agent_LT* Bid::getBidder() const
{
    return bidder;
}

void Bid::setWtpErrorTerm(double error)
{
	wtpErrorTerm = error;
}

double Bid::getWtpErrorTerm() const
{
	return wtpErrorTerm;
}


namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Bid& data) {
            return strm << "{"
						<< "\"unitId\":\"" << data.newUnitId << "\","
						<< "\"bidderId\":\"" << data.bidderId << "\","
						<< "\"bidValue\":\"" << data.bidValue << "\","
						<< "\"simulationDay\":\"" << data.simulationDay << "\""
						<< "}";
        }
    }
}
