//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   EventPublisher.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 7, 2013, 11:30 AM
 */

#include <boost/property_map/property_map.hpp>

#include "EventPublisher.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob::event;
using std::list;
using std::pair;

namespace {
    //Helper functions declaration.
    bool exitsEvent(const ContextListenersMap& map, EventId id);
    void remove(ListenersList& listenersList, EventListenerPtr listener);
    void publishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
            EventId id, Context ctx, const EventArgs& args);
    void subscribeListener(ContextListenersMap& map, EventId id, Context ctx,
            EventListenerPtr listener, Callback callback);
}

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
}

void EventPublisher::registerEvent(EventId id) {
    if (!exitsEvent(listeners, id)) {
        listeners.insert(pair<EventId, ContextMap>(id, ContextMap()));
    }
}

void EventPublisher::unRegisterEvent(EventId id) {
    if (exitsEvent(listeners, id)) {
        // remove all context listeners.
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap& cm = ctxMapItr->second;
            cm.clear();
        }
        //erase entries in 
        listeners.erase(id);
    }
}

bool EventPublisher::isEventRegistered(EventId id) const {
    return exitsEvent(listeners, id);
}

void EventPublisher::publish(EventId id, const EventArgs& args) {
    // publish using the global context.    
    publishEvent(listeners, true, this, id, this, args);
}

void EventPublisher::publish(EventId id, Context ctx, const EventArgs& args) {
    publishEvent(listeners, true, this, id, this, args);
    //notify context listeners.
    publishEvent(listeners, false, this, id, ctx, args);
}

void EventPublisher::subscribe(EventId id, EventListenerPtr listener,
        Context context) {
    subscribe(id, listener, &EventListener::onEvent, context);
}

void EventPublisher::subscribe(EventId id, EventListenerPtr listener,
        Callback callback, Context ctx) {
    if (exitsEvent(listeners, id) && listener && callback) {
        subscribeListener(listeners, id, (ctx) ? ctx : this,
                listener, callback);
    }
}

void EventPublisher::unSubscribe(EventId id, EventListenerPtr listener) {
    unSubscribe(id, this, listener);
}

void EventPublisher::unSubscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    if (exitsEvent(listeners, id) && listener) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            if (mapItr != ctxMapItr->second.end()) { //Context exists
                ListenersList& ll = mapItr->second;
                remove(ll, listener);
                //if list is empty then remove the context.
                if (ll.empty()) {
                    //remove the list
                    ctxMapItr->second.erase(ctx);
                }
            }
        }
    }
}

void EventPublisher::unSubscribeAll(EventListenerPtr listener) {
    if (listener) {
        //TODO: this method is very inefficient we need something better. But for now...
        for (ContextListenersMap::iterator itr = listeners.begin(); itr != listeners.end(); itr++) {
            ContextMap& cm = itr->second;
            for (ContextMap::iterator mapItr = cm.begin(); mapItr != cm.end(); mapItr++) {
                ListenersList& ll = mapItr->second;
                for (ListenersList::iterator listItr = ll.begin(); listItr != ll.end(); listItr++) {
                    Entry& entry = *listItr;
                    if (entry.listener == listener) {
                        listItr = ll.erase(listItr);
                        break;
                    }
                }
            }
        }
    }
}

void EventPublisher::unSubscribeAll(EventId id) {
    unSubscribeAll(id, this);
}

void EventPublisher::unSubscribeAll(EventId id, Context ctx) {
    if (exitsEvent(listeners, id)) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does exists
                ListenersList& ll = mapItr->second;
                ll.clear();
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
namespace {

    bool exitsEvent(const ContextListenersMap& map, EventId id) {
        return (map.find(id) != map.end());
    }

    void subscribeListener(ContextListenersMap& map, EventId id, Context ctx,
            EventListenerPtr listener, Callback callback) {
        ContextListenersMap::iterator ctxMapItr = map.find(id);
        if (ctxMapItr != map.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            ListenersList* ctxList = 0;
            if (mapItr == ctxMapItr->second.end()) { //Context Id does not exists
                std::pair < ContextMap::iterator, bool> ret;
                ret = ctxMapItr->second.insert(
                        pair<Context, ListenersList>(ctx, ListenersList()));
                ctxList = &(ret.first->second);
            } else {
                ctxList = &(mapItr->second);
            }
            ctxList->push_back(Entry(listener, callback));
        }
    }

    void publishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
            EventId id, Context ctx, const EventArgs& args) {
        //notify context listeners.
        ContextListenersMap::iterator ctxMapItr = map.find(id);
        if (ctxMapItr != map.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctx);
            if (mapItr != ctxMapItr->second.end()) { // Event ID is registered.
                ListenersList& lst = mapItr->second;
                ListenersList::iterator lstItr = lst.begin();
                while (lstItr != lst.end()) {
                    // notify listener
                    if (globalCtx) {
                        (((*lstItr).listener)->*((*lstItr).callback))(id, sender, sender, args);
                    } else {
                        (((*lstItr).listener)->*((*lstItr).callback))(id, ctx, sender, args);
                    }
                    lstItr++;
                }
            }
        }
    }

    void remove(ListenersList& listenersList, EventListenerPtr listener) {
        if (listener) {
            ListenersList::iterator lstItr = listenersList.begin();
            while (lstItr != listenersList.end()) {
                if ((*lstItr).listener == listener) {
                    Entry& entry = *lstItr;
                    listenersList.erase(lstItr);
                    return;
                }
                lstItr++;
            }
        }
    }
}
