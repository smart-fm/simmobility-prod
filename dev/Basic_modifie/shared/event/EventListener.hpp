//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventListener.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:37 AM
 */

#pragma once

#include "args/EventArgs.hpp"

namespace sim_mob {
    
    namespace event {

        class EventPublisher;

        ///A Conext is used to refine event subscriptions.
        ///It is essentially a "thing" (hence the void*) that
        /// is used only for identification (hence the const).
        typedef const void* Context;

        ///Interface for all event listener implementation.
        class EventListener {
        public:
            virtual ~EventListener() = 0;

            /**
             * Handles the received context event.
             * @param sender pointer for the event producer.
             * @param id event identifier.
             * @param args event arguments.
             */
            virtual void onEvent(sim_mob::event::EventId id,
                    sim_mob::event::Context ctxId,
                    sim_mob::event::EventPublisher* sender,
                    const EventArgs& args) {
            }
        };

        inline EventListener::~EventListener() {
        };

    }
}
