//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   GlobalParams.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on July 1, 2013, 3:04 PM
 */

#include "GlobalParams.hpp"

using namespace sim_mob::long_term;
using std::string;

GlobalParams::GlobalParams(BigSerial id, double unitTypeWeight,double unitAreaWeight,
        double unitRentWeight, double unitStoreyWeight) :
id(id), unitTypeWeight(unitTypeWeight), unitAreaWeight(unitAreaWeight), 
unitRentWeight(unitRentWeight), unitStoreyWeight(unitStoreyWeight){
}

GlobalParams::~GlobalParams() {
}

GlobalParams& GlobalParams::operator=(const GlobalParams& source) {
    this->id = source.id;
    this->unitAreaWeight = source.unitAreaWeight;
    this->unitRentWeight = source.unitRentWeight;
    this->unitStoreyWeight = source.unitStoreyWeight;
    this->unitTypeWeight = source.unitTypeWeight;
    return *this;
}

BigSerial GlobalParams::getId() const {
    return id;
}

double GlobalParams::getUnitTypeWeight() const {
    return unitTypeWeight;
}

double GlobalParams::getUnitRentWeight() const {
    return unitRentWeight;
}

double GlobalParams::getUnitAreaWeight() const {
    return unitAreaWeight;
}

double GlobalParams::getUnitStoreyWeight() const {
    return unitStoreyWeight;
}
