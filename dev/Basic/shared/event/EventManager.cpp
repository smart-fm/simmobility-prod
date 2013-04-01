/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventManager.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 1, 2013, 11:27 AM
 */

#include "EventManager.hpp"

using namespace sim_mob;
using std::map;
using std::list;
using std::pair;

EM_EventArgs::EM_EventArgs() : EventArgs(){}
EM_EventArgs::~EM_EventArgs(){}


EventManager::EventManager() : temporalWindows(TimesliceComparator()),
currTime(timeslice(0, 0)) {
    RegisterEvent(EM_WND_EXPIRED);
    RegisterEvent(EM_WND_UPDATED); // future...
}

EventManager::~EventManager() {
    temporalWindows.clear();
}

void EventManager::Update(const timeslice& currTime) {
    this->currTime = currTime;
    TemporalWindowMap::iterator itr = temporalWindows.find(currTime);
    if (itr != temporalWindows.end()) { //we have windows to update.
        for (TemporalWindowList::iterator litr = itr->second.begin();
                litr != itr->second.end(); litr++) {
            if (litr->IsExpired(currTime)) {//only to confirm
                Publish(EM_WND_EXPIRED, &(*litr), EventArgs());
                // remove all subscribers for this window and event.
                UnSubscribeAll(EM_WND_EXPIRED, &(*litr));
            }
        }
        //clears the entry. 
        itr->second.clear();
        temporalWindows.erase(currTime);
    }
}

void EventManager::Schedule(const timeslice& target, EventListenerPtr listener) {
    Schedule(target, listener, 0);
}

void EventManager::Schedule(const timeslice& target, EventListenerPtr listener,
        ListenerContextCallback callback) {
    TemporalWindowMap::iterator itr = temporalWindows.find(target);
    TemporalWindowList* listPtr = 0;
    if (itr == temporalWindows.end()) { // is not registered
        std::pair < TemporalWindowMap::iterator, bool> ret = temporalWindows.insert(
                pair<timeslice, TemporalWindowList>(target, TemporalWindowList()));
        listPtr = &ret.first->second;
    } else {
        listPtr = &itr->second;
    }
    TemporalWindow tw(currTime, timeslice(currTime.ms() + target.ms(),
            currTime.frame() + target.frame()));
    TemporalWindowList::iterator litr = listPtr->insert(listPtr->end(), tw);
    if (callback) {
        Subscribe(EM_WND_EXPIRED, &(*litr), listener, callback);
    } else {
        Subscribe(EM_WND_EXPIRED, &(*litr), listener);
    }
}

/**
 * 
 * TEMPORAL WINDOW.
 *  
 */
EventManager::TemporalWindow::TemporalWindow(const timeslice& from, const timeslice& to)
: from(from), to(to) {

}

EventManager::TemporalWindow::~TemporalWindow() {
}

bool EventManager::TemporalWindow::IsExpired(const timeslice& current) const {
    return (current.ms() > to.ms() ||
            (current.ms() == to.ms() && current.frame() >= to.frame()));
}

const timeslice& EventManager::TemporalWindow::GetTo() const {
    return to;
}

const timeslice& EventManager::TemporalWindow::GetFrom() const {
    return from;
}