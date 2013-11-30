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

EM_EventArgs::EM_EventArgs() : EventArgs() {
}

EM_EventArgs::~EM_EventArgs() {
}

EventManager::EventManager() : windows(TimeComparator()),
currTime(timeslice(0, 0)) {
    registerEvent(EM_WND_EXPIRED);
    registerEvent(EM_WND_UPDATED); // future...
}

EventManager::~EventManager() {
    for (WindowMap::iterator itr = windows.begin(); itr != windows.end(); 
            itr++) {
        WindowList& wList = itr->second;
        for (WindowList::iterator litr = wList.begin();
                litr != wList.end(); litr++) {
            TemporalWindow& window = *litr;
        }
        //clears the entry. 
        wList.clear();
    }
    windows.clear();
}

void EventManager::update(const timeslice& currTime) {

    this->currTime = currTime;

    {// synchronized scope
        upgrade_lock<shared_mutex> upgradeLock(windowsMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        WindowMap::iterator itr = windows.find(currTime);
        if (itr != windows.end()) { //we have windows to update.
            WindowList& wList = itr->second;
            for (WindowList::iterator litr = wList.begin();
                    litr != wList.end(); litr++) {
                TemporalWindow& window = *litr;
                if (window.isExpired(currTime)) {//only to confirm
                    publish(EM_WND_EXPIRED, &window, EventArgs());
                    // remove all subscribers for this window and event.
                    unSubscribeAll(EM_WND_EXPIRED, &window);
                }
                //clears the entry. 
                wList.clear();
                windows.erase(currTime);
            }
        }
    }
}

void EventManager::schedule(const timeslice& target,
        EventListenerPtr listener) {
    schedule(target, listener, nullptr);
}

void EventManager::schedule(const timeslice& target, EventListenerPtr listener,
        Callback callback) {

    {// synchronized scope
        upgrade_lock<shared_mutex> upgradeLock(windowsMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        WindowMap::iterator itr = windows.find(target);
        WindowList* listPtr = nullptr;
        if (itr == windows.end()) { // is not registered
            MapRetVal ret = windows.insert(MapEntry(target, WindowList()));
            listPtr = &(ret.first->second);
        } else {
            listPtr = &(itr->second);
        }
        TemporalWindow tw(currTime, target);
        WindowList::iterator litr = listPtr->insert(listPtr->end(), tw);
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
EventManager::TemporalWindow::TemporalWindow(const timeslice& from,
        const timeslice& to)
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
