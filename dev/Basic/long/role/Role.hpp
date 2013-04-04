/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Role.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 4:55 PM
 */
#pragma once

#include "entities/Agent.hpp"
#include "event/EventListener.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a generic Role that can be updated calling the method Update. 
         */
        class Role : public EventListener{
        public:
            Role(Agent* parent);
            virtual ~Role();

            /**
             * Method to implement the role behavior.
             * @param currTime
             */
            virtual void Update(timeslice currTime) = 0;
            /**
             * Tells if the role is active or not.
             * @return true if the role is active, false otherwise.
             */
            bool isActive() const;

            /**
             * Disable or enable the role.
             * @param active
             */
            void SetActive(bool active);
        
        protected:
            /**
             * Gets the role's parent.
             * @return Parent pointer.
             */
            Agent* GetParent() const;
            
        private:
            bool active;
            Agent* parent;
        };
    }
}

