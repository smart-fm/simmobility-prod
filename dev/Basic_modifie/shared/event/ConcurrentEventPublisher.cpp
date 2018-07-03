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

void ConcurrentEventPublisher::registerEvent(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::registerEvent(id);
    }
}

void ConcurrentEventPublisher::unRegisterEvent(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::unRegisterEvent(id);
    }
}

bool ConcurrentEventPublisher::isEventRegistered(EventId id) const {
    {
        shared_lock<shared_mutex> lock(listenersMutex);
        return EventPublisher::isEventRegistered(id);
    }
}

void ConcurrentEventPublisher::publish(EventId id, const EventArgs& args) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::publish(id, args);
    }
}

void ConcurrentEventPublisher::publish(EventId id, Context ctx, const EventArgs& args) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::publish(id, ctx, args);
    }
}

void ConcurrentEventPublisher::subscribe(EventId id, EventListenerPtr listener, 
                    Callback callback, Context context){
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::subscribe(id, listener, callback, context);
    }
}

void ConcurrentEventPublisher::unSubscribe(EventId id, EventListenerPtr listener) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::unSubscribe(id, listener);
    }

}

void ConcurrentEventPublisher::unSubscribe(EventId id, Context ctx,
        EventListenerPtr listener) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::unSubscribe(id, ctx, listener);
    }
}

void ConcurrentEventPublisher::unSubscribeAll(EventId id) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::unSubscribeAll(id);
    }
}

void ConcurrentEventPublisher::unSubscribeAll(EventId id, Context ctx) {
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(listenersMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        EventPublisher::unSubscribeAll(id, ctx);
    }
}
