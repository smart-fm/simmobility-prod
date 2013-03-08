/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventPublisher.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:30 AM
 */

#pragma once

#include <map>
#include <list>
#include "EventPublisher.hpp"

namespace sim_mob {

    using std::map;
    using std::list;
    typedef list<EventListenerPtr> ListenersList;
    typedef map<EventId, ListenersList > ListenersMap;

    /*
     * Generic implementation of event publisher.
     * 
     * This implementation is not thread-safe. 
     */
    class GenericEventPublisher : EventPublisher {
    public:
        GenericEventPublisher();
        virtual ~GenericEventPublisher();

        /**
         * Inherited from EventPublisher.
         * @param id of the event.
         * @param args arguments to pass to the listeners.
         */
        virtual void Notify(EventId id, const EventArgs& args);
        virtual void Subscribe(EventId id, EventListenerPtr);
        virtual void UnSubscribe(EventId id, EventListenerPtr);
        virtual void RegisterEvent(EventId id);
        virtual void UnRegisterEvent(EventId id);
        virtual bool IsEventRegistered(EventId id) const;

    private:
        /**
         * Verifies if given EventPublisher pointer already exists on list.
         * @param listData to verify.
         * @param ptr data to find.
         * @return true if exits, false otherwise.
         */
        bool Contains(const ListenersList& listData, EventListenerPtr ptr) const;
    private:
        ListenersMap listeners;
    };
}

