/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *  
 * File:   EventPublisher.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 7, 2013, 11:30 AM
 */

#include "EventPublisher.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;
using std::list;
using std::pair;

//Helper functions declaration.
void RemoveAll(ListenersList& listenersList);
void RemoveAll(ContextMap& listeners);
void RemoveAll(ContextListenersMap& map);
void Remove(ListenersList& listenersList, EventListenerPtr listener);
void PublishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
        EventId id, Context ctx, const EventArgs& args);
void SubscribeListener(ContextListenersMap& map, EventId id, Context ctx,
        EventListenerPtr listener, Callback callback);

/**********************
 * ListenerEntry
 **********************/
ListenerEntry::ListenerEntry(EventListenerPtr listener, Callback callback) {
    this->callback = callback;
    this->listener = listener;
}

/**********************
 * Event Publisher
 **********************/
EventPublisher::EventPublisher() {
}

EventPublisher::~EventPublisher() {
    // deletes all global listeners.
    RemoveAll(listeners);
}

void EventPublisher::RegisterEvent(EventId id) {
    if (!IsEventRegistered(id)) {
        listeners.insert(pair<EventId, ContextMap>(id, ContextMap()));
    }
}

void EventPublisher::UnRegisterEvent(EventId id) {
    if (IsEventRegistered(id)) {
        // remove all context listeners.
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            RemoveAll(ctxMapItr->second);
        }
        //erase entries in 
        listeners.erase(id);
    }
}

bool EventPublisher::IsEventRegistered(EventId id) const {
    return (listeners.find(id) != listeners.end());
}

void EventPublisher::Publish(EventId id, const EventArgs& args) {
    // publish using the global context.    
    PublishEvent(listeners, true, this, id, this, args);
}

void EventPublisher::Publish(EventId id, Context ctx, const EventArgs& args) {
    // first notify global listeners.
    Publish(id, args);
    //notify context listeners.
    PublishEvent(listeners, false, this, id, ctx, args);
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener) {
    Subscribe(id, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener,
        ListenerCallback callback) {
    if (IsEventRegistered(id) && listener && callback) {
        Callback cb;
        cb.callback = callback;
        SubscribeListener(listeners, id, this, listener, cb);
    }
}

void EventPublisher::Subscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    Subscribe(id, ctx, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, Context ctx,
        EventListenerPtr listener, ListenerContextCallback callback) {
    if (IsEventRegistered(id) && listener && callback) {
        Callback cb;
        cb.contextCallback = callback;
        SubscribeListener(listeners, id, ctx, listener, cb);
    }
}

void EventPublisher::UnSubscribe(EventId id, EventListenerPtr listener) {
    UnSubscribe(id, this, listener);
}

void EventPublisher::UnSubscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    if (IsEventRegistered(id) && listener) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does exists
                Remove(mapItr->second, listener);
                //if list is empty then remove the context.
                if (mapItr->second.empty()) {
                    ctxMapItr->second.erase(ctx);
                }
            }
        }
    }
}

void EventPublisher::UnSubscribeAll(EventId id) {
    if (IsEventRegistered(id)) {
        UnSubscribeAll(id, this);
    }
}

void EventPublisher::UnSubscribeAll(EventId id, Context ctx) {
    if (IsEventRegistered(id)) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does exists
                RemoveAll(mapItr->second);
                ctxMapItr->second.erase(ctx);
            }
        }
    }
}

/**********************
 * 
 * HELPER FUNCTIONS
 *
 **********************/
void SubscribeListener(ContextListenersMap& map, EventId id, Context ctx,
        EventListenerPtr listener, Callback callback) {
    ContextListenersMap::iterator ctxMapItr = map.find(id);
    if (ctxMapItr != map.end()) {
        ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
        ListenersList* ctxList = 0;
        if (mapItr == ctxMapItr->second.end()) { //Context Id does not exists
            std::pair < ContextMap::iterator, bool> ret;
            ret = ctxMapItr->second.insert(
                    pair<Context, ListenersList>(ctx, ListenersList()));
            ctxList = &ret.first->second;
        } else {
            ctxList = &mapItr->second;
        }
        ctxList->push_back(Entry(listener, callback));
    }
}

void PublishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
        EventId id, Context ctx, const EventArgs& args) {
    //notify context listeners.
    ContextListenersMap::iterator ctxMapItr = map.find(id);
    if (ctxMapItr != map.end()) {
        ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
        if (mapItr != ctxMapItr->second.end()) { // Event ID is registered.
            ListenersList lst = mapItr->second;
            ListenersList::iterator lstItr = lst.begin();
            while (lstItr != lst.end()) {
                // notify listener
                if (globalCtx) {
                    (((&*lstItr)->listener)->*((&*lstItr)->callback.callback))
                            (id, sender, args);
                } else {
                    (((&*lstItr)->listener)->*((&*lstItr)->callback.contextCallback))
                            (id, ctx, sender, args);
                }
                lstItr++;
            }
        }
    }
}

void RemoveAll(ContextMap& listeners) {
    for (ContextMap::iterator itr = listeners.begin(); itr != listeners.end();
            itr++) {
        RemoveAll(itr->second);
        itr->second.clear();
    }
    listeners.clear();
}

void RemoveAll(ContextListenersMap& map) {
    for (ContextListenersMap::iterator itr = map.begin(); itr != map.end();
            itr++) {
        RemoveAll(itr->second);
    }
    map.clear();
}

void RemoveAll(ListenersList& listenersList) {
    listenersList.clear();
}

void Remove(ListenersList& listenersList, EventListenerPtr listener) {
    if (listener) {
        ListenersList::iterator lstItr = listenersList.begin();
        while (lstItr != listenersList.end()) {
            if ((&*lstItr)->listener == listener) {
                listenersList.erase(lstItr);
                return;
            }
            lstItr++;
        }
    }
}
