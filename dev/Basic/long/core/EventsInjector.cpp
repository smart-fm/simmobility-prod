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

using namespace sim_mob;
using namespace sim_mob::long_term;
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

    vector<ExternalEvent> events;
    LuaProvider::getExternalEventsModel().getExternalEvents(now.ms(), events);
    vector<ExternalEvent>::iterator it = events.begin();
    for (it; it != events.end(); ++it) {
        PrintOut("External Event day:[" << it->day << "] hh:[" << it->householdId << "] type:[" << it->type << "]" << std::endl);
    }
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}
