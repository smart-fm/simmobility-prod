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
            (((*lstItr)->listener)->*((*lstItr)->callback.callback))(id, this, args);
            lstItr++;
        }
    }
}

void EventPublisher::Publish(EventId id, ContextId ctxId, const EventArgs& args) {
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
                (((*lstItr)->listener)->*((*lstItr)->callback.contextCallback))
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
            mapItr->second.push_back(new Entry(listener, cb));
        }
    }
}

void EventPublisher::Subscribe(EventId id, ContextId ctxId, EventListenerPtr listener) {
    Subscribe(id, ctxId, listener, &EventListener::OnEvent);
}

void EventPublisher::Subscribe(EventId id, ContextId ctxId, EventListenerPtr listener,
        ListenerContextCallback callback) {
    if (IsEventRegistered(id) && listener && callback) {
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            ContextMap::iterator mapItr = ctxMapItr->second.find(ctxId);
            ListenersList* ctxList = 0;
            if (mapItr == ctxMapItr->second.end()) { //Context Id does not exists
                std::pair < ListenersMap::iterator, bool> ret;
                ret = ctxMapItr->second.insert(
                        pair<ContextId, ListenersList>(ctxId, ListenersList()));
                ctxList = &ret.first->second;
            } else {
                ctxList = &mapItr->second;
            }
            Callback cb;
            cb.contextCallback = callback;
            ctxList->push_back(new Entry(listener, cb));
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

void EventPublisher::UnSubscribe(EventId id, ContextId ctxId, EventListenerPtr listener) {
    if (IsEventRegistered(id) && listener) {
        ContextListenersMap::iterator ctxMapItr = contextListeners.find(id);
        if (ctxMapItr != contextListeners.end()) {
            ListenersMap::iterator mapItr = ctxMapItr->second.find(ctxId);
            if (mapItr != ctxMapItr->second.end()) { //Context Id does not exists
                Remove(mapItr->second, listener);
            }
        }
    }
}

void EventPublisher::Remove(ListenersList& listenersList, EventListenerPtr listener) {
    if (listener) {
        ListenersList::iterator lstItr = listenersList.begin();
        Entry* entry = 0;
        while (lstItr != listenersList.end()) {
            if ((*lstItr)->listener == listener) {
                entry = (*lstItr);
                listenersList.erase(lstItr);
                // Does not delete the listener.
                safe_delete_item(entry);
                return;
            }
            lstItr++;
        }
    }
}

void EventPublisher::RemoveAll(ListenersList& listenersList) {
    ListenersList::iterator lstItr = listenersList.begin();
    while (lstItr != listenersList.end()) {
        safe_delete_item(*lstItr);
        lstItr++;
    }
}

void EventPublisher::RemoveAll(ListenersMap& listeners) {
    for (ListenersMap::iterator itr = listeners.begin();
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