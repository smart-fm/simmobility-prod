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

using namespace sim_mob;
using namespace sim_mob::long_term;


Unit::Unit(BigSerial id, BigSerial buildingId, BigSerial typeId, BigSerial postcodeId,
        BigSerial tazId, double floorArea, int storey, double rent, 
        double latitude, double longitude) :
id(id), buildingId(buildingId), typeId(typeId), postcodeId(postcodeId), 
tazId(tazId), storey(storey), floorArea(floorArea), rent(rent), 
location(latitude, longitude){}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->buildingId = source.buildingId;
    this->typeId = source.typeId;
    this->postcodeId = source.postcodeId;
    this->tazId = source.tazId;
    this->storey = source.storey;
    this->floorArea = source.floorArea;
    this->rent = source.rent;
    this->location.latitude = source.location.latitude;
    this->location.longitude = source.location.longitude;
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
    this->location.latitude = source.location.latitude;
    this->location.longitude = source.location.longitude;
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

const LatLngLocation& Unit::getLocation() const {
    return location;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Unit& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"buildingId\":\"" << data.buildingId << "\","
                    << "\"typeId\":\"" << data.typeId << "\","
                    << "\"postcodeId\":\"" << data.postcodeId << "\","
                    << "\"tazId\":\"" << data.tazId << "\","
                    << "\"floorArea\":\"" << data.floorArea << "\","
                    << "\"storey\":\"" << data.storey << "\","
                    << "\"rent\":\"" << data.rent << "\","
                    << "\"latitude\":\"" << data.location.latitude << "\","
                    << "\"longitude\":\"" << data.location.longitude << "\""
                    << "}";
        }
    }
}