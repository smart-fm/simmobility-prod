/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   UnitStateEventArgs.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:38 PM
 */
#pragma once
#include "event/EventArgs.hpp"

namespace sim_mob {
    
    namespace long_term {

        enum UnitState {
            Registered = 0,
            UnRegistered,
            Available,
            UnAvailable,
        };

        /**
         * Represents the arguments for all events 
         * related with the change of unit state on the market.
         */
        class UnitStateEventArgs : public EventArgs {
        public:
            UnitStateEventArgs(const UnitState& state, int unitId);
            UnitStateEventArgs(const UnitStateEventArgs& orig);
            virtual ~UnitStateEventArgs();
            
            /**
             * Gets the state of the unit on the market.
             */
            UnitState GetState() const;
            
            /**
             * Gets the id of the unit on the market.
             */
            int GetUnitId() const;
            
        private:
            UnitState state;
            int unitId;
        };
    }
}

