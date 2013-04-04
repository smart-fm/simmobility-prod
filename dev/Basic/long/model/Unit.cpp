/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

Unit::Unit(UnitId id) 
        : id(id){
}

Unit::Unit(const Unit& source) {
    this->id = source.id;
    this->available = source.available;
}

Unit::~Unit() {
}

Unit& Unit::operator=(const Unit& source) {
    this->id = source.id;
    this->available = source.available;
    return *this;
}

bool Unit::IsAvailable() const{
    return available;
}

UnitId Unit::GetId() const {
    return id;
}
