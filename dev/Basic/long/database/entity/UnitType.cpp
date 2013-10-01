//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   UnitType.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:04 PM
 */

#include "UnitType.hpp"

using namespace sim_mob::long_term;
using std::string;

UnitType::UnitType(BigSerial id, string name) :
id(id), name(name) {
}

UnitType::~UnitType() {
}

UnitType& UnitType::operator=(const UnitType& source) {
    this->id = source.id;
    this->name = source.name;
    return *this;
}

BigSerial UnitType::GetId() const {
    return id;
}

string UnitType::GetName() const {
    return name;
}
