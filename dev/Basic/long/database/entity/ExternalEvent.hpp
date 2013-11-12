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
            unsigned int getType() const; //int because lua
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

