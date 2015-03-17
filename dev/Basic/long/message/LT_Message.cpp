//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_Message.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 8, 2013, 11:06 AM
 */

#include "LT_Message.hpp"


using namespace sim_mob::messaging;
using namespace sim_mob::long_term;

BidMessage::BidMessage(const Bid& bid)
: bid(bid), response(NOT_ACCEPTED) {
}

BidMessage::BidMessage(const Bid& bid, BidResponse response)
: bid(bid), response(response) {
}

BidMessage::BidMessage(const BidMessage& source)
: bid(source.bid), response(source.response) {
}

BidMessage::~BidMessage() {
}

BidMessage& BidMessage::operator=(const BidMessage& source) {
    this->bid = source.bid;
    this->response = source.response;
    return *this;
}

const BidResponse& BidMessage::getResponse() const {
    return response;
}

const Bid& BidMessage::getBid() const {
    return bid;
}

HM_ActionMessage::HM_ActionMessage(BigSerial buildingId, std::tm futureDemolitionDate)
: unitId(INVALID_ID),buildingId(buildingId),buildingFutureDemolitionDate(buildingFutureDemolitionDate) ,unit(nullptr),building(nullptr){
}

HM_ActionMessage::HM_ActionMessage(const HM_ActionMessage& source)
: unitId(source.unitId),buildingId(source.buildingId), buildingFutureDemolitionDate(source.buildingFutureDemolitionDate),unit(unit),building(building){
}

HM_ActionMessage::HM_ActionMessage(Unit &unit)
: unitId(unit.getId()),buildingId(unit.getBuildingId()), buildingFutureDemolitionDate(std::tm()), unit(&unit),building(nullptr){
}

HM_ActionMessage::HM_ActionMessage(Building &building)
: unitId(0),buildingId(building.getFmBuildingId()), buildingFutureDemolitionDate(std::tm()), unit(nullptr),building(&building){
}

HM_ActionMessage::HM_ActionMessage(BigSerial unitId)
:unitId(unitId),buildingId(INVALID_ID), buildingFutureDemolitionDate(std::tm()), unit(nullptr),building(nullptr){

}

HM_ActionMessage::~HM_ActionMessage() {
}

BigSerial HM_ActionMessage::getUnitId() const {
    return unitId;
}

BigSerial HM_ActionMessage::getBuildingId() const {
    return buildingId;
}

std::tm HM_ActionMessage::getFutureDemolitionDate() const {
	return buildingFutureDemolitionDate;
}

Unit *HM_ActionMessage::getUnit() const
{
	return unit;
}

Building  *HM_ActionMessage::getBuilding() const
{
	return building;
}
