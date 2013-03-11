/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *  
 * File:   EventPublisher.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 7, 2013, 11:30 AM
 */

#include "GenericEventPublisher.hpp"

using namespace sim_mob;
using std::list;
using std::find;
using std::pair;

GenericEventPublisher::GenericEventPublisher() {
}

GenericEventPublisher::~GenericEventPublisher() {
    listeners.clear();
}

void GenericEventPublisher::Notify(EventId id, const EventArgs& args) {
    ListenersMap::iterator mapItr = listeners.find(id);
    if (mapItr != listeners.end()) { // Event ID is registered.
        ListenersList lst = mapItr->second;
        ListenersList::iterator lstItr = lst.begin();
        while (lstItr != lst.end()) {
            // notify listener
            (*lstItr)->OnEvent(this, id, args);
            lstItr++;
        }
    }
}

void GenericEventPublisher::Subscribe(EventId id, EventListenerPtr listener) {
    ListenersMap::iterator mapItr = listeners.find(id);
    if (mapItr != listeners.end()) { // Event ID is registered.
        if (!Contains(mapItr->second, listener)) {
            mapItr->second.push_front(listener);
        }
    }
}

void GenericEventPublisher::UnSubscribe(EventId id, EventListenerPtr listener) {
    ListenersMap::iterator mapItr = listeners.find(id);
    if (mapItr != listeners.end()) { // Event ID is registered.
        mapItr->second.remove(listener);
    }
}

void GenericEventPublisher::RegisterEvent(EventId id) {
    if (!IsEventRegistered(id)) {
        listeners.insert(pair<EventId, ListenersList>(id, ListenersList()));
    }
}

void GenericEventPublisher::UnRegisterEvent(EventId id) {
    if (IsEventRegistered(id)) {
        listeners.erase(id);
    }
}

bool GenericEventPublisher::IsEventRegistered(EventId id) const {
    return (listeners.find(id) != listeners.end());
}

bool GenericEventPublisher::Contains(const ListenersList& listData, EventListenerPtr ptr) const {
    //@TODO: linear search change the data structure to a Linked Hash Map.
    return !(find(listData.begin(), listData.end(), ptr) == listData.end());
}
