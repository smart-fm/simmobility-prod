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
#include "EventListener.hpp"

namespace sim_mob {

    using std::map;
    using std::list;

    typedef EventListener* EventListenerPtr;
    typedef EventListener::EventCallback ListenerCallback;
    typedef EventListener::EventContextCallback ListenerContextCallback;

    union Callback {
        ListenerCallback callback;
        ListenerContextCallback contextCallback;
    };

    /**
     * Struct to store a Listener entry.
     */
    typedef struct ListenerEntry {
        ListenerEntry(EventListenerPtr listener, Callback callback);
        EventListenerPtr listener;
        Callback callback;
    } Entry;

    // map for global listeners.
    typedef list<Entry> ListenersList;
    typedef map<EventId, ListenersList> ListenersMap;
    // maps for listeners with context.
    typedef map<Context, ListenersList> ContextMap;
    typedef map<EventId, ContextMap> ContextListenersMap;

    /**
     * Generic implementation of event publisher.
     * 
     * This implementation is not thread-safe. 
     */
    class EventPublisher {
    public:
        EventPublisher();
        virtual ~EventPublisher();

        /**
         * Registers a new event id on publisher.
         * @param id EventId to register.
         */
        virtual void RegisterEvent(EventId id);
        /**
         * UnRegisters the given event from the publisher. 
         * @param id EventId to un-register.
         */
        virtual void UnRegisterEvent(EventId id);

        /**
         * Verifies if register exists.
         * @param id
         * @return 
         */
        virtual bool IsEventRegistered(EventId id) const;

        /**
         * Publishes an event with given EventId.
         * @param id to publish.
         * @param args of the event.
         */
        virtual void Publish(EventId id, const EventArgs& args);

        /**
         * Publishes an event with given EventId and ContextId.
         * @param id to publish.
         * @param ctxId Id of the context.
         * @param args of the event.
         */
        virtual void Publish(EventId id, Context ctxId, const EventArgs& args);

        /**
         * 
         * Subscribes the given global listener to the given EventId.
         * This listener will receive all events of the given EventId.
         * 
         * @param id
         * @param listener to subscribe.
         */
        virtual void Subscribe(EventId id, EventListenerPtr listener);

        /**
         * Subscribes the given global listener to the given EventId.
         * This listener will receive all events of the given EventId.
         * 
         * The given callback that called will be called instead of normal OnEvent
         * 
         * @param id
         * @param listener to subscribe.
         */
        virtual void Subscribe(EventId id, EventListenerPtr listener,
                ListenerCallback eventFunction);

        /**
         * Subscribes the given listener to the given EventId and ContextId.
         * This listener will receive only the events with given EventId and
         * fired for the given contextId.
         * @param id of the event.
         * @param ctxId Id of the context.
         * @param listener to subscribe.
         */
        virtual void Subscribe(EventId id, Context ctxId, EventListenerPtr listener);

        /**
         * Subscribes the given listener to the given EventId and ContextId.
         * This listener will receive only the events with given EventId and
         * fired for the given contextId.
         * 
         * The given callback that called will be called instead of normal OnEvent
         * 
         * @param id of the event.
         * @param ctxId Id of the context.
         * @param listener to subscribe.
         */
        virtual void Subscribe(EventId id, Context ctxId,
                EventListenerPtr listener, ListenerContextCallback eventFunction);

        /**
         * UnSubscribes the given listener to the given EventId
         * 
         * ATTENTION: the listener instance will not be deleted.
         * 
         * @param id of the event.
         * @param listener to UnSubscribe.
         */
        virtual void UnSubscribe(EventId id, EventListenerPtr listener);

        /**
         * UnSubscribes the given listener to the given EventId and ContextId.
         * 
         * ATTENTION: the listener instance will not be deleted.
         * 
         * @param id of the event.
         * @param ctxId Id of the context.
         * @param listener to UnSubscribe.
         */
        virtual void UnSubscribe(EventId id, Context ctxId,
                EventListenerPtr listener);

        /**
         * UnSubscribes all global subscribers for the given event id.  
         * @param id of the global event.
         */
        void UnSubscribeAll(EventId id);
        /**
         * UnSubscribes all context subscribers for the given event id and context.  
         * @param id of the event.
         * @param ctx context.
         */
        void UnSubscribeAll(EventId id, Context ctx);

    private:
        /**
         * Removes listeners from given list.
         * @param listenersList of listeners to update.
         * @param listener to remove. 
         */
        void Remove(ListenersList& listenersList, EventListenerPtr listener);

        /**
         * Removes all listeners from given list.
         * @param listenersList of listeners to update.
         * @param listener to remove. 
         */
        void RemoveAll(ListenersList& listenersList);

        /**
         * Removes listeners from given map.
         * @param map of listeners to remove.
         */
        void RemoveAll(ListenersMap& listeners);

        /**
         * Removes listeners from given map.
         * @param map of listeners to remove.
         */
        void RemoveAll(ContextMap& listeners);

        /**
         * Removes all listeners from given context map.
         * @param map of listeners to remove.
         */
        void RemoveAll(ContextListenersMap& map);

    private:
        ContextListenersMap contextListeners;
        ListenersMap globalListeners;
    };
}

