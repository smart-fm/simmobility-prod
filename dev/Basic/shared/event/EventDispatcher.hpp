/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventDispatcher.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 10:15 AM
 */
#pragma once

#include <map>
#include "EventPublisher.hpp"

namespace sim_mob {

    using std::map;
    using std::pair;
    typedef map<EventPublisher*, EventPublisher*> Publishers;
    typedef pair<EventPublisher*, EventPublisher*> PublishersPair;

    class EventDispatcher {
    public:

        EventDispatcher();
        virtual ~EventDispatcher();

        /**
         * Defines the application main listener.
         * The main listener will receive on the application 
         * all events from all threads. The main listener is 
         * responsible for sending the event back to the
         * EventDispatcher from the main thread.
         * @param listener Event listener
         * @note To be called once by the application
         */
        void SetMainListener(EventListener* listener);

        /**
         * Registers a new publisher.
         * Attention: This class does not manage the life
         * cycle of given EventPublisher instance.
         * Please you need to guarantee that you call the 
         * UnRegister before the instance destruction.
         * @param publisher pointer of EventPublisher instance to register.
         */
        void RegisterPublisher(EventPublisher* publisher);

        /**
         * UnRegisters a given publisher.
         * @param publisher pointer of EventPublisher instance to unregister.
         */
        void UnRegisterPublisher(EventPublisher* publisher);

        /**
         * Verifies if given EventPublisher is registered.
         * @param publisher pointer of EventPublisher instance to verify.
         * @return true if is registered, false otherwise.
         */
        bool IsPublisherRegistered(EventPublisher* publisher) const;

        /**
         * Dispatches the given event using the given publisher
         * and EventId to all registered listeners.
         * @param publisher pointer of EventPublisher instance to notify.
         * @param id of the event to call.
         * @param args of the event.
         */
        void Dispatch(EventPublisher* publisher, EventId id, const EventArgs& args);

        /**
         * Dispatches the given event using the given 
         * publisher and EventId to all registered listeners.
         * Attention: This method can be called by any thread
         * and will deliver the event asynchronously.
         * This method will use the mainListener to pass 
         * events (from any thread) to the main thread.
         * This can be useful when we have,for example, GUI frameworks.
         * 
         * If the main listener is not defined the function will not
         * fire any event.
         * 
         * @param publisher pointer of EventPublisher instance to notify.
         * @param id of the event to call.
         * @param args of the event.
         */
        void DispatchAsync(EventPublisher* publisher, EventId id, const EventArgs& args);
    private:
        EventListener* mainListener;
        Publishers publishers;
    };
}

