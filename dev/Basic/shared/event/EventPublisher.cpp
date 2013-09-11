/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *  
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
    bool ExitsEvent(const ContextListenersMap& map, EventId id);
    void RemoveAll(ListenersList& listenersList);
    void RemoveAll(ContextMap& listeners);
    void RemoveAll(ContextListenersMap& map);
    void Remove(ListenersList& listenersList, EventListenerPtr listener);
    void PublishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
            EventId id, Context ctx, const EventArgs& args);
    void SubscribeListener(ContextListenersMap& map, EventId id, Context ctx,
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
    RemoveAll(listeners);
}

void EventPublisher::RegisterEvent(EventId id) {
    if (!ExitsEvent(listeners, id)) {
        listeners.insert(pair<EventId, ContextMap*>(id, new ContextMap()));
    }
}

void EventPublisher::UnRegisterEvent(EventId id) {
    if (ExitsEvent(listeners, id)) {
        // remove all context listeners.
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap* cm = ctxMapItr->second;
            RemoveAll(*cm);
            safe_delete_item(cm);
        }
        //erase entries in 
        listeners.erase(id);
    }
}

bool EventPublisher::IsEventRegistered(EventId id) const {
    return ExitsEvent(listeners, id);
}

void EventPublisher::Publish(EventId id, const EventArgs& args) {
    // publish using the global context.    
    PublishEvent(listeners, true, this, id, this, args);
}

void EventPublisher::Publish(EventId id, Context ctx, const EventArgs& args) {
    PublishEvent(listeners, true, this, id, this, args);
    //notify context listeners.
    PublishEvent(listeners, false, this, id, ctx, args);
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener) {
    Subscribe(id, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener,
        ListenerCallback callback) {
    if (ExitsEvent(listeners, id) && listener && callback) {
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
    if (ExitsEvent(listeners, id) && listener && callback) {
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
    if (ExitsEvent(listeners, id) && listener) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second->find(ctx);
            if (mapItr != ctxMapItr->second->end()) { //Context exists
                ListenersList* ll = mapItr->second;
                Remove(*ll, listener);
                //if list is empty then remove the context.
                if (ll->empty()) {
                    //remove the list
                    safe_delete_item(ll);
                    ctxMapItr->second->erase(ctx);
                }
            }
        }
    }
}

void EventPublisher::UnSubscribeAll(EventListenerPtr listener) {
    if (listener) {
        //TODO: this method is very inefficient we need something better. But for now...
        for (ContextListenersMap::iterator itr = listeners.begin(); itr != listeners.end(); itr++) {
            ContextMap* cm = itr->second;
            for (ContextMap::iterator mapItr = cm->begin(); mapItr != cm->end(); mapItr++) {
                ListenersList* ll = mapItr->second;
                for (ListenersList::iterator listItr = ll->begin(); listItr != ll->end(); listItr++) {
                    Entry* entry = *listItr;
                    if (entry->listener == listener) {
                        listItr = ll->erase(listItr);
                        safe_delete_item(entry);
                        break;
                    }
                }
            }
        }
    }
}
            
void EventPublisher::UnSubscribeAll(EventId id) {
    UnSubscribeAll(id, this);
}

void EventPublisher::UnSubscribeAll(EventId id, Context ctx) {
    if (ExitsEvent(listeners, id)) {
        ContextListenersMap::iterator ctxMapItr = listeners.find(id);
        if (ctxMapItr != listeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second->find(ctx);
            if (mapItr != ctxMapItr->second->end()) { //Context Id does exists
                ListenersList* ll = mapItr->second;
                RemoveAll(*ll);
                safe_delete_item(ll);
                ctxMapItr->second->erase(ctx);
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

    bool ExitsEvent(const ContextListenersMap& map, EventId id) {
        return (map.find(id) != map.end());
    }

    void SubscribeListener(ContextListenersMap& map, EventId id, Context ctx,
            EventListenerPtr listener, Callback callback) {
        ContextListenersMap::iterator ctxMapItr = map.find(id);
        if (ctxMapItr != map.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second->find(ctx);
            ListenersList* ctxList = 0;
            if (mapItr == ctxMapItr->second->end()) { //Context Id does not exists
                std::pair < ContextMap::iterator, bool> ret;
                ret = ctxMapItr->second->insert(
                        pair<Context, ListenersList*>(ctx, new ListenersList()));
                ctxList = ret.first->second;
            } else {
                ctxList = mapItr->second;
            }
            ctxList->push_back(new Entry(listener, callback));
        }
    }

    void PublishEvent(ContextListenersMap& map, bool globalCtx, EventPublisher* sender,
            EventId id, Context ctx, const EventArgs& args) {
        //notify context listeners.
        ContextListenersMap::iterator ctxMapItr = map.find(id);
        if (ctxMapItr != map.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second->find(ctx);
            if (mapItr != ctxMapItr->second->end()) { // Event ID is registered.
                ListenersList* lst = mapItr->second;
                ListenersList::iterator lstItr = lst->begin();
                while (lstItr != lst->end()) {
                    // notify listener
                    if (globalCtx) {
                        (((*lstItr)->listener)->*((*lstItr)->callback.callback))(id, sender, args);
                    } else {
                        (((*lstItr)->listener)->*((*lstItr)->callback.contextCallback))(id, ctx, sender, args);
                    }
                    lstItr++;
                }
            }
        }
    }

    void RemoveAll(ContextMap& listeners) {
        for (ContextMap::iterator itr = listeners.begin(); itr != listeners.end();
                itr++) {
            ListenersList* ll = itr->second;
            RemoveAll(*ll);
            safe_delete_item(ll);
        }
        listeners.clear();
    }

    void RemoveAll(ContextListenersMap& map) {
        for (ContextListenersMap::iterator itr = map.begin(); itr != map.end();
                itr++) {
            ContextMap* cm = itr->second;
            RemoveAll(*cm);
            safe_delete_item(cm);
        }
        map.clear();
    }

    void RemoveAll(ListenersList& listenersList) {
        for (ListenersList::iterator itr = listenersList.begin();
                itr != listenersList.end(); itr++) {
            Entry* entry = *itr;
            safe_delete_item(entry);
        }
        listenersList.clear();
    }

    void Remove(ListenersList& listenersList, EventListenerPtr listener) {
        if (listener) {
            ListenersList::iterator lstItr = listenersList.begin();
            while (lstItr != listenersList.end()) {
                if ((*lstItr)->listener == listener) {
                    Entry* entry = *lstItr;
                    safe_delete_item(entry);
                    listenersList.erase(lstItr);
                    return;
                }
                lstItr++;
            }
        }
    }
}