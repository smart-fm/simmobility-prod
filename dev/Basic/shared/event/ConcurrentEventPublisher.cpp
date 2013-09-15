//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   ConcurrentEventPublisher.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on August 25, 2013, 11:30 AM
 */

#include <boost/property_map/property_map.hpp>

#include "ConcurrentEventPublisher.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob::event;
using boost::shared_mutex;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::upgrade_to_unique_lock;

ConcurrentEventPublisher::ConcurrentEventPublisher() {
}

ConcurrentEventPublisher::~ConcurrentEventPublisher() {
}

void ConcurrentEventPublisher::RegisterEvent(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::RegisterEvent(id);
    }
}

void ConcurrentEventPublisher::UnRegisterEvent(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::UnRegisterEvent(id);
    }
}

bool ConcurrentEventPublisher::IsEventRegistered(EventId id) const {
    {
        shared_lock<shared_mutex> lock(listenersMutex);
        return EventPublisher::IsEventRegistered(id);
    }
}

void ConcurrentEventPublisher::Publish(EventId id, const EventArgs& args) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Publish(id, args);
    }
}

void ConcurrentEventPublisher::Publish(EventId id, Context ctx, const EventArgs& args) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Publish(id, ctx, args);
    }
}

void ConcurrentEventPublisher::Subscribe(EventId id, EventListenerPtr listener) {
    {// thread-safe scope

        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Subscribe(id, listener);
    }
}

void ConcurrentEventPublisher::Subscribe(EventId id, EventListenerPtr listener,
        ListenerCallback callback) {
    {// thread-safe scope

        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Subscribe(id, listener, callback);
    }
}

void ConcurrentEventPublisher::Subscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    {// thread-safe scope

        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Subscribe(id, ctx, listener);
    }
}

void ConcurrentEventPublisher::Subscribe(EventId id, Context ctx,
        EventListenerPtr listener, ListenerContextCallback callback) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::Subscribe(id, ctx, listener, callback);
    }
}

void ConcurrentEventPublisher::UnSubscribe(EventId id, EventListenerPtr listener) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::UnSubscribe(id, listener);
    }

}

void ConcurrentEventPublisher::UnSubscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::UnSubscribe(id, ctx, listener);
    }
}

void ConcurrentEventPublisher::UnSubscribeAll(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::UnSubscribeAll(id);
    }
}

void ConcurrentEventPublisher::UnSubscribeAll(EventId id, Context ctx) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::UnSubscribeAll(id, ctx);
    }
}
