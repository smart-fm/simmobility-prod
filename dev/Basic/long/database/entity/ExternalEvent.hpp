/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   ExternalEvent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 12, 2013, 2:51 PM
 */
#pragma once
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Entity that represents an External event.
         * 
         * An external event can be:
         * - New job,
         * - New job location 
         * - New school location
         *  etc..
         * 
         * Normally these events are injected to perform some variance 
         * in the simulation. 
         * 
         */
        class ExternalEvent {
        public:

            enum Type {
                NEW_JOB = 0,
                LOST_JOB,
                NEW_JOB_LOCATION,
                NEW_CHILD,
                NEW_SCHOOL_LOCATION,
                UNKNOWN,
            };
            
        public:
            ExternalEvent();
            ExternalEvent(const ExternalEvent& orig);
            virtual ~ExternalEvent();

            /**
             * Getters 
             */
            int getDay() const;
            unsigned int getType() const; //int because lua does not support enums
            BigSerial getHouseholdId() const;
            
            /**
             * Setters 
             */
            void setDay(int day);
            void setType(unsigned int type);
            void setHouseholdId(BigSerial id);

        private:
            int day;
            Type type;
            BigSerial householdId;
        };
    }
}

