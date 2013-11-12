/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventsInjector.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 8, 2013, 1:32 PM
 */

#include "EventsInjector.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/MessageBus.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;

using std::vector;

EventsInjector::EventsInjector() : Entity(-1) {
}

EventsInjector::~EventsInjector() {
}

bool EventsInjector::isNonspatial() {
    return false;
}

void EventsInjector::buildSubscriptionList(vector<BufferedBase*>& subsList) {
}

void EventsInjector::onWorkerEnter() {
}

void EventsInjector::onWorkerExit() {
}

Entity::UpdateStatus EventsInjector::update(timeslice now) {
    const ExternalEventsModel& model = LuaProvider::getExternalEventsModel();
    
    vector<ExternalEvent> events;
    //(now+1) - events for the next day once our events are 1 tick delayed
    model.getExternalEvents((now.ms() + 1), events);
    vector<ExternalEvent>::iterator it = events.begin();
    for (it; it != events.end(); ++it) {
        PrintOut("External Event day:[" << it->getDay() << "] hh:[" << it->getHouseholdId() << "] type:[" << it->getType() << "]" << std::endl);
        //MessageBus::PublishEvent();
    }
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}
