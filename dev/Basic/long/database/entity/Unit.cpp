/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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

namespace {
    double CalculateHedonicPrice(const Unit& unit) {
        return unit.GetRent(); /*(double) unit.GetFixedPrice() +
            unit.GetArea() * unit.GetWeightArea() +
            (((int) unit.GetType()) + 1) * unit.GetWeightType() +
            unit.GetStorey() * unit.GetWeightStorey() +
            unit.GetLastRemodulationYear() * unit.GetWeightYearLastRemodulation() +
            unit.GetTaxExempt() * unit.GetWeightTaxExempt() +
            unit.GetDistanceToCBD() * unit.GetWeightDistanceToCBD();*/
    }
}

Unit::Unit(UnitId id, BigSerial buildingId, BigSerial typeId,
        double area, int storey, double rent, bool available) :
id(id), buildingId(buildingId), typeId(typeId),
storey(storey), area(area), rent(rent), available(available), 
askingPrice(0), hedonicPrice(0), owner(nullptr) {}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->storey = source.storey;
    this->area = source.area;
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
    this->storey = source.storey;
    this->area = source.area;
    this->rent = source.rent;
    this->available = source.available;
    this->askingPrice = source.askingPrice;
    this->hedonicPrice = source.hedonicPrice;
    this->owner = source.owner;
    return *this;
}

UnitId Unit::GetId() const {
    return id;
}

BigSerial Unit::GetBuildingId() const {
    return buildingId;
}

BigSerial Unit::GetTypeId() const {
    return typeId;
}

int Unit::GetStorey() const {
    return storey;
}

double Unit::GetArea() const {
    return area;
}

double Unit::GetRent() const {
    return rent;
}

bool Unit::IsAvailable() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return available;
}

void Unit::SetAvailable(bool avaliable) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->available = avaliable;
}

double Unit::GetAskingPrice() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return askingPrice;
}
double Unit::GetHedonicPrice() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return hedonicPrice;
}

void Unit::SetAskingPrice(double askingPrice) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    askingPrice = askingPrice;
}

void Unit::SetHedonicPrice(double hedonicPrice) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    hedonicPrice = hedonicPrice;
}

void Unit::SetOwner(UnitHolder* receiver) {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    this->owner = receiver;
}

UnitHolder* Unit::GetOwner() {
    boost::upgrade_lock<boost::shared_mutex> upLock(mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> lock(upLock);
    //TODO: this is not protecting nothing.
    return this->owner;
}