//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventManager.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 1, 2013, 11:27 AM
 */

#include "EventManager.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob::event;
using boost::shared_mutex;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::upgrade_to_unique_lock;
using std::map;
using std::list;
using std::pair;

EM_EventArgs::EM_EventArgs() : EventArgs(){}
EM_EventArgs::~EM_EventArgs(){}


EventManager::EventManager() : temporalWindows(TimesliceComparator()),
currTime(timeslice(0, 0)) {
    registerEvent(EM_WND_EXPIRED);
    registerEvent(EM_WND_UPDATED); // future...
}

EventManager::~EventManager() {
    for (TemporalWindowMap::iterator itr = temporalWindows.begin();
            itr != temporalWindows.end(); itr++) {
        TemporalWindowList& wList = itr->second;
        for (TemporalWindowList::iterator litr = wList.begin();
                litr != wList.end(); litr++) {
            TemporalWindow& window = *litr;
        }
        //clears the entry. 
        wList.clear();
    }
    temporalWindows.clear();
}

void EventManager::update(const timeslice& currTime) {
    
    this->currTime = currTime;
    
    {// synchronized scope
        upgrade_lock<shared_mutex> upgradeLock(windowsMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        TemporalWindowMap::iterator itr = temporalWindows.find(currTime);
        if (itr != temporalWindows.end()) { //we have windows to update.
            TemporalWindowList& wList = itr->second;
            for (TemporalWindowList::iterator litr = wList.begin();
                    litr != wList.end(); litr++) {
                TemporalWindow& window = *litr;
                if (window.isExpired(currTime)) {//only to confirm
                    publish(EM_WND_EXPIRED, &window, EventArgs());
                    // remove all subscribers for this window and event.
                    unSubscribeAll(EM_WND_EXPIRED, &window);
                }
                //clears the entry. 
                wList.clear();
                temporalWindows.erase(currTime);
            }
        }
    }
}

void EventManager::schedule(const timeslice& target, EventListenerPtr listener) {
    schedule(target, listener, nullptr);
}

void EventManager::schedule(const timeslice& target, EventListenerPtr listener,
        Callback callback) {

    {// synchronized scope
    	upgrade_lock<shared_mutex> upgradeLock(windowsMutex);
    	upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        TemporalWindowMap::iterator itr = temporalWindows.find(target);
        TemporalWindowList* listPtr = nullptr;
        if (itr == temporalWindows.end()) { // is not registered
            std::pair < TemporalWindowMap::iterator, bool> ret = temporalWindows.insert(
                    pair<timeslice, TemporalWindowList>(target, TemporalWindowList()));
            listPtr = &(ret.first->second);
        } else {
            listPtr = &(itr->second);
        }
        TemporalWindow tw (currTime, target);
        TemporalWindowList::iterator litr = listPtr->insert(listPtr->end(), tw);
        if (callback) {
            subscribe(EM_WND_EXPIRED, listener, callback, &(*litr));
        } else {
            subscribe(EM_WND_EXPIRED, listener, &(*litr));
        }
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

bool EventManager::TemporalWindow::isExpired(const timeslice& current) const {
    return (current.ms() > to.ms() ||
            (current.ms() == to.ms() && current.frame() >= to.frame()));
}

const timeslice& EventManager::TemporalWindow::getTo() const {
    return to;
}

const timeslice& EventManager::TemporalWindow::getFrom() const {
    return from;
}
