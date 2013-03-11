/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventDispatcher.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 10:15 AM
 */

#include "EventDispatcher.hpp"

using namespace sim_mob;
namespace sim_mob {

    EventDispatcher::EventDispatcher()
    : mainListener(NULL) {
    }

    EventDispatcher::~EventDispatcher() {
        if (mainListener) {
            mainListener = NULL;
        }
    }

    void EventDispatcher::SetMainListener(sim_mob::EventListener* listener) {
        mainListener = listener;
    }

    void EventDispatcher::RegisterPublisher(sim_mob::EventPublisher* publisher) {
        if (!IsPublisherRegistered(publisher)) {
            publishers.insert(PublishersPair(publisher, publisher));
        }
    }

    void EventDispatcher::UnRegisterPublisher(sim_mob::EventPublisher* publisher) {
        if (IsPublisherRegistered(publisher)) {
            publishers.erase(publisher);
        }
    }

    void EventDispatcher::Dispatch(sim_mob::EventPublisher* publisher, EventId id, const EventArgs& args) {
        if (IsPublisherRegistered(publisher)) {
            publisher->Notify(id, args);
        }
    }

    void EventDispatcher::DispatchAsync(sim_mob::EventPublisher* publisher, EventId id, const EventArgs& args) {
        if (IsPublisherRegistered(publisher) && mainListener) {
            mainListener->OnEvent(publisher, id, args);
        }
    }

    bool EventDispatcher::IsPublisherRegistered(sim_mob::EventPublisher* publisher) const {
        return (publisher && (publishers.find(publisher) != publishers.end()));
    }
}
