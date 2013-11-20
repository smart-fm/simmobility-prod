/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   ExternalEvent.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 12, 2013, 2:51 PM
 */

#include "ExternalEvent.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

ExternalEvent::ExternalEvent()
: day(0), type(ExternalEvent::UNKNOWN), householdId(INVALID_ID) {
}

ExternalEvent::ExternalEvent(const ExternalEvent& orig) {
    this->day = orig.day;
    this->householdId = orig.householdId;
    this->type = orig.type;
}

ExternalEvent::~ExternalEvent() {
}

int ExternalEvent::ExternalEvent::getDay() const {
    return day;
}

unsigned int ExternalEvent::ExternalEvent::getType() const {
    return (unsigned int)type;
}

BigSerial ExternalEvent::getHouseholdId() const {
    return householdId;
}

void ExternalEvent::setDay(int day) {
    this->day = day;
}

void ExternalEvent::setType(unsigned int type) {
    this->type = (type < ExternalEvent::UNKNOWN) ?
            (ExternalEvent::Type)type : ExternalEvent::UNKNOWN;
}

void ExternalEvent::setHouseholdId(BigSerial id) {
    this->householdId = id;
}