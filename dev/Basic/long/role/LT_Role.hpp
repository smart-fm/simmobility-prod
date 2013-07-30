/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Role.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 4:55 PM
 */
#pragma once

//Boost has a solution for is_base_of, but it takes 4 lines to use.
//#include <boost/type_traits.hpp>

#include "event/EventListener.hpp"
#include "agent/LT_Agent.hpp"

//TEMP: Needed for logging.
#include "workers/Worker.hpp"


namespace sim_mob {

    namespace long_term {

        /**
         * Represents a generic role that can be updated during the simulation.
         * 
         * Each Long-Term agent can have N roles and act using 
         * each one for different things like:
         * - Selling Units
         * - Bidding new Units
         * - Selling lands
         * - Bidding lands
         * - etc...
         */
        class LT_Role {
        public:

            LT_Role() : active(false) {
            }

            virtual ~LT_Role() {
            }

            /**
             * Method to implement the role behavior.
             * @param currTime
             */
            virtual void Update(timeslice currTime) = 0;

            /**
             * Tells if the role is active or not.
             * @return true if the role is active, false otherwise.
             */
            bool isActive() const {
                return active;
            }

            /**
             * Disable or enable the role.
             * @param active
             */
            void SetActive(bool active) {
                this->active = active;
            }

            /**
             * Handle messages.
             * @param type of the message.
             * @param sender of the message.
             * @param message data.
             */
            virtual void HandleMessage(messaging::MessageReceiver::MessageType type,
                    messaging::MessageReceiver& sender, const messaging::Message& message) {
            }

        private:
            bool active;
        };

        /**
         * Represents a Generic Agent Role template implementation.
         * Template class should be a concrete LT_Agent class.
         */
        template<typename T> class LT_AgentRole : public LT_Role, public sim_mob::event::EventListener {
        public:

            LT_AgentRole(T* parent) : parent(parent) {
            	//Ensure that parent is a subclass of type Agent. (Won't compile otherwise).
            	//TODO: This is only for the sake of the Log() function.
            	Agent* parentAg = parent;
            }

            virtual ~LT_AgentRole() {
            }

        protected:

            /**
             * Gets the role's parent.
             * @return Parent pointer.
             */
            T* GetParent() const {
                return parent;
            }

            //Allow LT_Roles to access the Logging infrastructure.
            //TODO: Might want to have LT_Role subclass Role so that we don't have to do a messy dynamic cast here.
            //      Templates make this more difficult than it should be.
            NullableOutputStream Log() {
            	return NullableOutputStream(dynamic_cast<Agent*>(parent)->currWorker->getLogFile());
            }

        private:
            T* parent;
        };
    }
}

