/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_EventArgs.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 5:42 PM
 */
#pragma once
#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"
#include "model/Bid.hpp"
#include "Common.h"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents an action of the market.
         */
        enum HM_Action {
            UNIT_ADDED,
            UNIT_REMOVED
        };

        DECLARE_CUSTOM_CALLBACK_TYPE(HM_EventArgs)
        class HM_ActionEventArgs : public EventArgs {
        public:
            HM_ActionEventArgs(HM_Action action, UnitId unitId);
            HM_ActionEventArgs(const HM_ActionEventArgs& orig);
            virtual ~HM_ActionEventArgs();

            /**
             * Gets the action that was performed by the market.
             * @return 
             */
            const HM_Action GetAction() const;

            /**
             * Gets the unit affected by the action.
             * @return 
             */
            const UnitId GetUnitId()const;
        private:
            HM_Action action;
            UnitId unitId;
        };
    }
}

