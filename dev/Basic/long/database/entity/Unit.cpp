//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"
#include "util/UnitHolder.hpp"
#include "util/Utils.hpp"
#include <boost/thread.hpp>

using namespace sim_mob::long_term;

Unit::Unit(UnitId id, BigSerial buildingId, BigSerial typeId, BigSerial postcodeId,
        double floorArea, int storey, double rent, bool available) :
id(id), buildingId(buildingId), typeId(typeId), postcodeId(postcodeId),
storey(storey), floorArea(floorArea), rent(rent), available(available), 
askingPrice(0), hedonicPrice(0), owner(nullptr) {}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
    this->available = source.available;
    this->askingPrice = source.askingPrice;
    this->hedonicPrice = source.hedonicPrice;
    this->owner = source.owner;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
    this->available = source.available;
    this->askingPrice = source.askingPrice;
    this->hedonicPrice = source.hedonicPrice;
    this->owner = source.owner;
    return *this;
}

UnitId Unit::getId() const {
    return id;
}

BigSerial Unit::getBuildingId() const {
    return buildingId;
}

BigSerial Unit::getTypeId() const {
    return typeId;
}

BigSerial Unit::getPostcodeId() const{
    return postcodeId;
}

int Unit::getStorey() const {
    return storey;
}

double Unit::getFloorArea() const {
    return floorArea;
}

double Unit::getRent() const {
    return rent;
}

bool Unit::isAvailable() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return available;
}

void Unit::setAvailable(bool avaliable) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->available = avaliable;
}

double Unit::getAskingPrice() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return this->askingPrice;
}
double Unit::getHedonicPrice() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return this->hedonicPrice;
}

void Unit::setAskingPrice(double askingPrice) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->askingPrice = askingPrice;
}

void Unit::setHedonicPrice(double hedonicPrice) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->hedonicPrice = hedonicPrice;
}

void Unit::setOwner(UnitHolder* receiver) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->owner = receiver;
}

UnitHolder* Unit::getOwner() {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    //TODO: this is not protecting nothing.
    return this->owner;
}
