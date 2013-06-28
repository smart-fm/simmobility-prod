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

#define WEIGHT_MIN 0.0f
#define WEIGHT_MAX 1.0f

double CalculateHedonicPrice(const Unit& unit) {
    return 0; /*(double) unit.GetFixedPrice() +
            unit.GetArea() * unit.GetWeightArea() +
            (((int) unit.GetType()) + 1) * unit.GetWeightType() +
            unit.GetStorey() * unit.GetWeightStorey() +
            unit.GetLastRemodulationYear() * unit.GetWeightYearLastRemodulation() +
            unit.GetTaxExempt() * unit.GetWeightTaxExempt() +
            unit.GetDistanceToCBD() * unit.GetWeightDistanceToCBD();*/
}

Unit::Unit(UnitId id, BigSerial buildingId, BigSerial establishmentId,
        BigSerial typeId, double area, int storey, double rent, bool available) :
id(id), buildingId(buildingId), establishmentId(establishmentId), typeId(typeId),
storey(storey), area(area), rent(rent), available(available), owner(nullptr) {
    //hedonicPrice = CalculateHedonicPrice(*this);
    //reservationPrice = hedonicPrice;
}

Unit::Unit() : id(INVALID_ID), buildingId(INVALID_ID), establishmentId(INVALID_ID),
typeId(INVALID_ID), storey(0), area(0), rent(0), available(false),
owner(nullptr) {
}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->establishmentId = source.establishmentId;
    this->typeId = source.typeId;
    this->storey = source.storey;
    this->area = source.area;
    this->rent = source.rent;
    this->available = source.available;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->establishmentId = source.establishmentId;
    this->typeId = source.typeId;
    this->storey = source.storey;
    this->area = source.area;
    this->rent = source.rent;
    this->available = source.available;
    return *this;
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

UnitId Unit::GetId() const {
    return id;
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

BigSerial Unit::GetBuildingId() const {
    return buildingId;
}

BigSerial Unit::GetEstablishmentId() const {
    return establishmentId;
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