/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Role.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 4:55 PM
 */
#pragma once

#include "event/EventListener.hpp"
#include "agent/LT_Agent.hpp"


namespace sim_mob {

    namespace long_term {
        
        /**
         * Represents a generic Role that can be updated during the simulation.
         * 
         * Each Long-Term agent can have N roles and act using 
         * each one for different things like:
         * - Selling Units
         * - Bid new Units
         * 
         */
        class LT_Role : public EventListener{
        public:
            LT_Role(LT_Agent* parent);
            virtual ~LT_Role();

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
             * Handle messages.
             * @param type of the message.
             * @param sender of the message.
             * @param message data.
             */
            virtual void HandleMessage(MessageType type, 
                    MessageReceiver& sender, const Message& message);
            /**
             * Gets the role's parent.
             * @return Parent pointer.
             */
            LT_Agent* GetParent() const;
            
        private:
            friend class LT_Agent;
            bool active;
            LT_Agent* parent;
        };
    }
}

