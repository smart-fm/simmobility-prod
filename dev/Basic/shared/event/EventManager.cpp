/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventManager.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 1, 2013, 11:27 AM
 */

#include "EventManager.hpp"
#include "util/LangHelpers.hpp"

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
    for (TemporalWindowMap::iterator itr = temporalWindows.begin();
            itr != temporalWindows.end(); itr++) {
        TemporalWindowList* wList = itr->second;
        for (TemporalWindowList::iterator litr = wList->begin();
                litr != wList->end(); litr++) {
            TemporalWindow* window = *litr;
            safe_delete_item(window);
        }
        //clears the entry. 
        wList->clear();
        safe_delete_item(wList);
    }
    temporalWindows.clear();
}

void EventManager::Update(const timeslice& currTime) {
    
    this->currTime = currTime;
    
    {// synchronized scope
    	boost::upgrade_lock<boost::shared_mutex> up_lock(windowsMutex);
    	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
        TemporalWindowMap::iterator itr = temporalWindows.find(currTime);
        if (itr != temporalWindows.end()) { //we have windows to update.
            TemporalWindowList* wList = itr->second;
            if (wList) {
                for (TemporalWindowList::iterator litr = wList->begin();
                        litr != wList->end(); litr++) {
                    TemporalWindow* window = *litr;
                    if (window && window->IsExpired(currTime)) {//only to confirm
                        Publish(EM_WND_EXPIRED, window, EventArgs());
                        // remove all subscribers for this window and event.
                        UnSubscribeAll(EM_WND_EXPIRED, window);
                        safe_delete_item(window);
                    }
                }
                //clears the entry. 
                wList->clear();
                temporalWindows.erase(currTime);
                safe_delete_item(wList);
            }
        }
    }
}

void EventManager::Schedule(const timeslice& target, EventListenerPtr listener) {
    Schedule(target, listener, nullptr);
}

void EventManager::Schedule(const timeslice& target, EventListenerPtr listener,
        ListenerContextCallback callback) {

    {// synchronized scope
    	boost::upgrade_lock<boost::shared_mutex> up_lock(windowsMutex);
    	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
        TemporalWindowMap::iterator itr = temporalWindows.find(target);
        TemporalWindowList* listPtr = nullptr;
        if (itr == temporalWindows.end()) { // is not registered
            std::pair < TemporalWindowMap::iterator, bool> ret = temporalWindows.insert(
                    pair<timeslice, TemporalWindowList*>(target, new TemporalWindowList()));
            listPtr = ret.first->second;
        } else {
            listPtr = itr->second;
        }
        TemporalWindow* tw = new TemporalWindow(currTime, target);
        TemporalWindowList::iterator litr = listPtr->insert(listPtr->end(), tw);
        if (callback) {
            Subscribe(EM_WND_EXPIRED, *litr, listener, callback);
        } else {
            Subscribe(EM_WND_EXPIRED, *litr, listener);
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
