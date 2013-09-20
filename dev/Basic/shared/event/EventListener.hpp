//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventListener.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:37 AM
 */

#include "args/EventArgs.hpp"


#pragma once

namespace sim_mob {

    namespace event {
        
        typedef void* Context;
        class EventPublisher;

        /**
         * Interface for all event listener implementation.
         */
        class EventListener {
        public:
            
            virtual ~EventListener()=0;
            
            /**
             * Handles the received global event.
             * @param sender pointer for the event producer.
             * @param id event identifier.
             * @param args event arguments.
             */
            virtual void OnEvent(sim_mob::event::EventId id, 
                                 sim_mob::event::EventPublisher* sender, 
                                 const EventArgs& args) {
            };

            /**
             * Handles the received context event.
             * @param sender pointer for the event producer.
             * @param id event identifier.
             * @param args event arguments.
             */
            virtual void OnEvent(sim_mob::event::EventId id, 
                                 sim_mob::event::Context ctxId, 
                                 sim_mob::event::EventPublisher* sender, 
                                 const EventArgs& args) {
            };

            /**
             * Functions for calls.
             */
            typedef void (EventListener::*EventCallback)(
                sim_mob::event::EventId id, 
                sim_mob::event::EventPublisher* sender, 
                const EventArgs& args);
            typedef void (EventListener::*EventContextCallback)(
                sim_mob::event::EventId id, 
                sim_mob::event::Context ctxId, 
                sim_mob::event::EventPublisher* sender, 
                const sim_mob::event::EventArgs& args);
        };
        
        inline EventListener::~EventListener(){};
    }
}
/**
 * Call this before you EventArgs implementation.
 */
#define DECLARE_CUSTOM_CALLBACK_TYPE(type) class type; \
        typedef void (sim_mob::event::EventListener::*type##Callback)(sim_mob::event::EventId id, sim_mob::event::EventPublisher* sender, const type& args); \
        typedef void (sim_mob::event::EventListener::*type##ContextCallback)(sim_mob::event::EventId id, sim_mob::event::Context ctxId, sim_mob::event::EventPublisher* sender, const type& args); 

/**
 * Call to pass your handler to the Publisher.
 * Example: Subscribe(evt3, sub3, CALLBACK_HANDLER(MyArgs, Subscriber::OnMyArgs));
 */
#define CALLBACK_HANDLER(type, func) (sim_mob::event::EventListener::EventCallback)(type##Callback) &func
#define CONTEXT_CALLBACK_HANDLER(type, func) (sim_mob::event::EventListener::EventContextCallback)(type##ContextCallback) &func
