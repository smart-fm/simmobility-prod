/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventsInjector.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 8, 2013, 1:32 PM
 */

#include "EventsInjector.hpp"

using namespace sim_mob;
using namespace sim_mob::long_term;
using std::vector;

EventsInjector::EventsInjector() : Entity(-2){
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
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}
