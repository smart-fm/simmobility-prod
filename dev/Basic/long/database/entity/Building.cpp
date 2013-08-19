/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 3:04 PM
 */

#include "Building.hpp"

using namespace sim_mob::long_term;

Building::Building(BigSerial id, BigSerial typeId,
        BigSerial parcelId, int builtYear,
        double floorArea, int storeys, int parkingSpaces) :
id(id), typeId(typeId), parcelId(parcelId), builtYear(builtYear),
floorArea(floorArea), storeys(storeys), parkingSpaces(parkingSpaces) {
}

Building::~Building() {
}

Building& Building::operator=(const Building& source) {
    this->id = source.id;
    this->typeId = source.typeId;
    this->parcelId = source.parcelId;
    this->builtYear = source.builtYear;
    this->floorArea = source.floorArea;
    this->parkingSpaces = source.parkingSpaces;
    this->storeys = source.storeys;
    return *this;
}

BigSerial Building::GetId() const {
    return id;
}

BigSerial Building::GetTypeId() const {
    return typeId;
}

BigSerial Building::GetParcelId() const {
    return parcelId;
}

int Building::GetBuiltYear() const {
    return builtYear;
}

double Building::GetFloorArea() const {
    return floorArea;
}

int Building::GetStoreys() const {
    return storeys;
}

int Building::GetParkingSpaces() const {
    return parkingSpaces;
}
