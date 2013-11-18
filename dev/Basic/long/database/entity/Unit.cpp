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

using namespace sim_mob::long_term;

Unit::Unit(BigSerial id, BigSerial buildingId, BigSerial typeId, BigSerial postcodeId,
        BigSerial tazId, double floorArea, int storey, double rent, 
        double latitude, double longitude) :
id(id), buildingId(buildingId), typeId(typeId), postcodeId(postcodeId), 
tazId(tazId), storey(storey), floorArea(floorArea), rent(rent), 
latitude(latitude), longitude(longitude) {}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->tazId = source.tazId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
    this->latitude = source.latitude;
    this->longitude = source.longitude;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->tazId = source.tazId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
    this->latitude = source.latitude;
    this->longitude = source.longitude;
    return *this;
}

BigSerial Unit::getId() const {
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

BigSerial Unit::getTazId() const{
    return tazId;
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

double Unit::getLatitude() const {
    return latitude;
}

double Unit::getLongitude() const {
    return longitude;
}