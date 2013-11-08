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

BuildingType::BuildingType(BigSerial id, const string& name) :
id(id), name(name){
}

BuildingType::~BuildingType() {
}

BuildingType& BuildingType::operator=(const BuildingType& source) {
    this->id = source.id;
    this->name = source.name;
    return *this;
}

BigSerial BuildingType::getId() const {
    return id;
}

const string& BuildingType::getName() const {
    return name;
}