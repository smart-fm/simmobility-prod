//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Postcode.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Feb 11, 2014, 3:05 PM
 */

#include "Postcode.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Postcode::Postcode(BigSerial id, BigSerial tazId, const std::string& code, 
                   double latitude, double longitude) 
: id(INVALID_ID), code(""), tazId(INVALID_ID){
}
            
Postcode::Postcode(const Postcode& source) {
    this->id = source.id;
    this->code = source.code;
    this->tazId = source.tazId;
    this->location.latitude = source.location.latitude;
    this->location.longitude = source.location.longitude;
}

Postcode::~Postcode() {
}

Postcode& Postcode::operator=(const Postcode& source) {
    this->id = source.id;
    this->code = source.code;
    this->tazId = source.tazId;
    this->location.latitude = source.location.latitude;
    this->location.longitude = source.location.longitude;
    return *this;
}

BigSerial Postcode::getId() const {
    return id;
}

const std::string& Postcode::getCode() const {
    return code;
}

BigSerial Postcode::getTazId() const {
    return tazId;
}

const LatLngLocation& Postcode::getLocation() const {
    return location;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Postcode& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"code\":\"" << data.code << "\","
                    << "\"tazId\":\"" << data.tazId << "\","
                    << "\"latitude\":\"" << data.location.latitude << "\","
                    << "\"longitude\":\"" << data.location.longitude << "\""
                    << "}";
        }
    }
}