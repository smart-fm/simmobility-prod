/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   UnitStateEventArgs.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 4:38 PM
 */

#include "UnitStateEventArgs.h"

using namespace sim_mob;
using namespace sim_mob::long_term;

namespace sim_mob {
    
    namespace long_term {

        UnitStateEventArgs::UnitStateEventArgs(const UnitState& state, int unitId)
        : EventArgs(), unitId(unitId), state(state) {
        }

        UnitStateEventArgs::UnitStateEventArgs(const UnitStateEventArgs& orig) {
            this->state = orig.state;
            this->unitId = orig.unitId;
        }

        UnitStateEventArgs::~UnitStateEventArgs() {
        }

        UnitState UnitStateEventArgs::GetState() const {
            return state;
        }

        int UnitStateEventArgs::GetUnitId() const {
            return unitId;
        }
    }
}
