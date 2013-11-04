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
        double floorArea, int storey, double rent) :
id(id), buildingId(buildingId), typeId(typeId), postcodeId(postcodeId),
storey(storey), floorArea(floorArea), rent(rent) {}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
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

int Unit::getStorey() const {
    return storey;
}

double Unit::getFloorArea() const {
    return floorArea;
}

double Unit::getRent() const {
    return rent;
}