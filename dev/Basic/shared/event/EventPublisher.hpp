/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventPublisher.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:30 AM
 */

#pragma once

#include "EventListener.hpp"
#include "EventArgs.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"


namespace sim_mob {

    typedef EventListener* EventListenerPtr;
    
    /*
     * Interface for all event publisher implementations.
     *
     */
    class EventPublisher {
    public:

        /**
         * Notifies all subscribers.
         * @param id of the event.
         * @param args with event data.
         */
        virtual void Notify(EventId id, const EventArgs& args) = 0;
        /**
         * Subscribes a new EventListener instance.
         * @param id of the event. 
         * @param Pointer to the EventListener instance. 
         */
        virtual void Subscribe(EventId id, EventListenerPtr ptr) = 0;
        /**
         * UnSubscribes a existing EventListener instance.
         * @param id of the event.
         * @param Pointer to the EventListener instance. 
         */
        virtual void UnSubscribe(EventId id, EventListenerPtr ptr) = 0;

        /**
         * Registers a new event type to handle.
         * @param id of the event.
         */
        virtual void RegisterEvent(EventId id) = 0;

        /**
         * UnRegisters the given event type.
         * @param id of the event.
         */
        virtual void UnRegisterEvent(EventId id) = 0;

        /**
         * Verifies if event is registered on this publisher.
         * @param id of the event.
         * @return true if is registered, false otherwise.
         */
        virtual bool IsEventRegistered(EventId id) const = 0;
    };
}

