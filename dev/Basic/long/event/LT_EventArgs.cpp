/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_EventArgs.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 4, 2013, 5:42 PM
 */

#include "LT_EventArgs.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;

HM_ActionEventArgs::HM_ActionEventArgs(UnitId unitId)
: unitId(unitId) {
}

HM_ActionEventArgs::HM_ActionEventArgs(const HM_ActionEventArgs& source)
: unitId(source.unitId) {
}

HM_ActionEventArgs::~HM_ActionEventArgs() {
}

const UnitId HM_ActionEventArgs::GetUnitId() const {
    return unitId;
}