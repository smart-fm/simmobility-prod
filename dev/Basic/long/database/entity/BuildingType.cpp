//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Building.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:04 PM
 */

#include "BuildingType.hpp"

using namespace sim_mob::long_term;
using std::string;

BuildingType::BuildingType(BigSerial id, string name, int type) :
id(id), name(name), type(type) {
}

BuildingType::~BuildingType() {
}

BuildingType& BuildingType::operator=(const BuildingType& source) {
    this->id = source.id;
    this->name = source.name;
    this->type = source.type;
    return *this;
}

BigSerial BuildingType::GetId() const {
    return id;
}

string BuildingType::GetName() const {
    return name;
}

int BuildingType::GetType() const {
    return type;
}
