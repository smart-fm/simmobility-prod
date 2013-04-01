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

ListenerEntry::ListenerEntry(EventListenerPtr listener, Callback callback) {
    this->callback = callback;
    this->listener = listener;
}

EventPublisher::EventPublisher() {
}

EventPublisher::~EventPublisher() {
    // deletes all global listeners.
    RemoveAll(globalListeners);
    // deletes all context listeners.
    RemoveAll(contextListeners);
}

void EventPublisher::RegisterEvent(EventId id) {
    if (!IsEventRegistered(id)) {
        globalListeners.insert(pair<EventId, ListenersList>(id, ListenersList()));
        contextListeners.insert(pair<EventId, ContextMap>(id, ContextMap()));
    }
}

void EventPublisher::UnRegisterEvent(EventId id) {
    if (IsEventRegistered(id)) {
        //removes all global listeners
        ListenersMap::iterator mapItr = globalListeners.find(id);
        if (mapItr != globalListeners.end()) {
            RemoveAll(mapItr->second);
        }
        // remove all context listeners.
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            RemoveAll(ctxMapItr->second);
        }

        globalListeners.erase(id);
        contextListeners.erase(id);
    }
}

bool EventPublisher::IsEventRegistered(EventId id) const {
    return (globalListeners.find(id) != globalListeners.end() &&
            contextListeners.find(id) != contextListeners.end());
}

void EventPublisher::Publish(EventId id, const EventArgs& args) {
    ListenersMap::iterator mapItr = globalListeners.find(id);
    if (mapItr != globalListeners.end()) { // Event ID is registered.
        ListenersList lst = mapItr->second;
        ListenersList::iterator lstItr = lst.begin();
        while (lstItr != lst.end()) {
            // notify listener
            (((&*lstItr)->listener)->*((&*lstItr)->callback.callback))
                    (id, this, args);
            lstItr++;
        }
    }
}

void EventPublisher::Publish(EventId id, Context ctxId, const EventArgs& args) {
    // first notify global listeners.
    Publish(id, args);
    //notify context listeners.
    ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
    if (ctxMapItr != contextListeners.end()) {
        ContextMap::iterator mapItr = ctxMapItr->second.find(ctxId);
        if (mapItr != ctxMapItr->second.end()) { // Event ID is registered.
            ListenersList lst = mapItr->second;
            ListenersList::iterator lstItr = lst.begin();
            while (lstItr != lst.end()) {
                // notify listener
                (((&*lstItr)->listener)->*((&*lstItr)->callback.contextCallback))
                        (id, ctxId, this, args);
                lstItr++;
            }
        }
    }
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener) {
    Subscribe(id, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, EventListenerPtr listener,
        ListenerCallback callback) {
    if (IsEventRegistered(id) && listener && callback) {
        ListenersMap::iterator mapItr = globalListeners.find(id);
        if (mapItr != globalListeners.end()) { // Event ID is registered.
            Callback cb;
            cb.callback = callback;
            mapItr->second.push_back(Entry(listener, cb));
        }
    }
}

void EventPublisher::Subscribe(EventId id, Context ctxId, EventListenerPtr listener) {
    Subscribe(id, ctxId, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, Context ctxId, EventListenerPtr listener,
        ListenerContextCallback callback) {
    if (IsEventRegistered(id) && listener && callback) {
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctxId);
            ListenersList* ctxList = 0;
            if (mapItr == ctxMapItr->second.end()) { //Context Id does not exists
                std::pair < ContextMap::iterator, bool> ret;
                ret = ctxMapItr->second.insert(
                        pair<Context, ListenersList>(ctxId, ListenersList()));
                ctxList = &ret.first->second;
            } else {
                ctxList = &mapItr->second;
            }
            Callback cb;
            cb.contextCallback = callback;
            ctxList->push_back(Entry(listener, cb));
        }
    }
}

void EventPublisher::UnSubscribe(EventId id, EventListenerPtr listener) {
    if (IsEventRegistered(id) && listener) {
        ListenersMap::iterator mapItr = globalListeners.find(id);
        if (mapItr != globalListeners.end()) { // Event ID is registered.
            Remove(mapItr->second, listener);
        }
    }
}

void EventPublisher::UnSubscribe(EventId id, Context ctxId, EventListenerPtr listener) {
    if (IsEventRegistered(id) && listener) {
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctxId);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does exists
                Remove(mapItr->second, listener);
                //if list is empty then remove the context.
                if (mapItr->second.empty()) {
                    ctxMapItr->second.erase(ctxId);
                }
            }
        }
    }
}

void EventPublisher::UnSubscribeAll(EventId id) {
    if (IsEventRegistered(id)) {
        ListenersMap::iterator mapItr = globalListeners.find(id);
        if (mapItr != globalListeners.end()) { // Event ID is registered.
            RemoveAll(mapItr->second);
        }
    }
}

void EventPublisher::UnSubscribeAll(EventId id, Context ctxId) {
    if (IsEventRegistered(id)) {
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctxId);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does exists
                RemoveAll(mapItr->second);
                ctxMapItr->second.erase(ctxId);
            }
        }
    }
}

void EventPublisher::Remove(ListenersList& listenersList, EventListenerPtr listener) {
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

void EventPublisher::RemoveAll(ListenersList& listenersList) {
    listenersList.clear();
}

void EventPublisher::RemoveAll(ListenersMap& listeners) {
    for (ListenersMap::iterator itr = listeners.begin();
            itr != listeners.end(); itr++) {
        RemoveAll(itr->second);
        itr->second.clear();
    }
    listeners.clear();
}

void EventPublisher::RemoveAll(ContextMap& listeners) {
    for (ContextMap::iterator itr = listeners.begin();
            itr != listeners.end(); itr++) {
        RemoveAll(itr->second);
        itr->second.clear();
    }
    listeners.clear();
}

void EventPublisher::RemoveAll(ContextListenersMap& map) {
    for (ContextListenersMap::iterator itr = map.begin();
            itr != map.end(); itr++) {
        RemoveAll(itr->second);
    }
    map.clear();
}