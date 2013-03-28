/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventListener.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:37 AM
 */

#include "EventArgs.hpp"


#pragma once

namespace sim_mob {

    class EventPublisher;
    typedef unsigned int EventId;
    typedef unsigned int ContextId;

    /**
     * Interface for all event listener implementation.
     */
    class EventListener {
    public:

        /**
         * Handles the received global event.
         * @param sender pointer for the event producer.
         * @param id event identifier.
         * @param args event arguments.
         */
        virtual void OnEvent(EventId id, EventPublisher* sender, const EventArgs& args) {
        };

        /**
         * Handles the received context event.
         * @param sender pointer for the event producer.
         * @param id event identifier.
         * @param args event arguments.
         */
        virtual void OnEvent(EventId id, ContextId ctxId, EventPublisher* sender, const EventArgs& args) {
        };

        /**
         * Functions for calls.
         */
        typedef void (EventListener::*EventCallback)(EventId id, EventPublisher* sender, const EventArgs& args);
        typedef void (EventListener::*EventContextCallback)(EventId id, ContextId ctxId, EventPublisher* sender, const EventArgs& args);
    };

    /**
     * Call this before you EventArgs implementation.
     */
#define DECLARE_CUSTOM_CALLBACK_TYPE(type) class type; \
        typedef void (EventListener::*type##Callback)(EventId id, EventPublisher* sender, const type& args); \
        typedef void (EventListener::*type##ContextCallback)(EventId id, ContextId ctxId, EventPublisher* sender, const type& args); 

    /**
     * Call to pass your handler to the Publisher.
     * Example: Subscribe(evt3, sub3, CALLBACK_HANDLER(MyArgs, Subscriber::OnMyArgs));
     */
#define CALLBACK_HANDLER(type, func) (EventListener::EventCallback)(type##Callback) &func
#define CONTEXT_CALLBACK_HANDLER(type, func) (EventListener::EventContextCallback)(type##ContextCallback) &func
}