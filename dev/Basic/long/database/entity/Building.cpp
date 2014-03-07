//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 8, 2013, 3:04 PM
 */

#include "Building.hpp"

using namespace sim_mob::long_term;

Building::Building(BigSerial id, BigSerial typeId, BigSerial parcelId, 
        BigSerial tenureId, int builtYear, double landedArea, int storeys, 
        int parkingSpaces) :
id(id), typeId(typeId), parcelId(parcelId), tenureId(tenureId), 
builtYear(builtYear), landedArea(landedArea), storeys(storeys), 
parkingSpaces(parkingSpaces) {
}

Building::~Building() {
}

Building& Building::operator=(const Building& source) {
    this->id = source.id;
    this->typeId = source.typeId;
    this->parcelId = source.parcelId;
    this->tenureId = source.tenureId;
    this->builtYear = source.builtYear;
    this->parkingSpaces = source.parkingSpaces;
    this->storeys = source.storeys;
    this->landedArea = source.landedArea;
    return *this;
}

BigSerial Building::getId() const {
    return id;
}

BigSerial Building::getTypeId() const {
    return typeId;
}

BigSerial Building::getParcelId() const {
    return parcelId;
}

BigSerial Building::getTenureId() const {
    return tenureId;
}

int Building::getBuiltYear() const {
    return builtYear;
}

int Building::getStoreys() const {
    return storeys;
}

int Building::getParkingSpaces() const {
    return parkingSpaces;
}

double Building::getLandedArea() const {
    return landedArea;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Building& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"typeId\":\"" << data.typeId << "\","
                    << "\"parcelId\":\"" << data.parcelId << "\","
                    << "\"tenureId\":\"" << data.tenureId << "\","
                    << "\"builtYear\":\"" << data.builtYear << "\","
                    << "\"storeys\":\"" << data.storeys << "\","
                    << "\"parkingSpaces\":\"" << data.parkingSpaces << "\","
                    << "\"landArea\":\"" << data.landedArea << "\""
                    << "}";
        }
    }
}