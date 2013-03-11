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

    // @TODO: put this in a global header.
    typedef const unsigned int EventId;

    /**
     * Interface for all event listener implementation.
     */
    class EventListener {
    public:
        /**
         * Handles the received event.
         * @param sender pointer for the event producer.
         * @param id event identifier.
         * @param args event arguments.
         */
        virtual void OnEvent(EventPublisher* sender, EventId id, const EventArgs& args) = 0;
    };
}

