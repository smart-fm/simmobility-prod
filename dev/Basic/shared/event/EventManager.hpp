
/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventManager.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 1, 2013, 11:27 AM
 */
#pragma once
#include "EventPublisher.hpp"
#include "metrics/Frame.hpp"

/**
 * All external events must start with this id.
 */
#define EM_EVENT_ID_START 10000

// Events for HousingMarket
#define EM_WND_EXPIRED          0
#define EM_WND_UPDATED          1

namespace sim_mob {

    using std::map;
    using std::list;

    DECLARE_CUSTOM_CALLBACK_TYPE(EM_EventArgs)
    class EM_EventArgs : public EventArgs {
    public:
        EM_EventArgs();
        virtual ~EM_EventArgs();
    };
    

    /**
     * Event Publisher that allows time-based events. 
     */
    class EventManager : public EventPublisher {
    public:
        EventManager();
        virtual ~EventManager();

        /**
         * Method to be called on which simulation update.
         * @param currTime current simulation time.
         */
        void Update(const timeslice& currTime);

        /**
         * Will create a temporal window from current ticket 
         * to (current tick + given ticks).
         * Each created window in this method will fire 2 events:
         *  - EM_WINDOW_UPDATE (only if the window is updateable)
         *  - EM_WINDOW_EXPIRED
         *
         * Attention: The listener will not be deleted by manager. 
         * @param target time to schedule.
         * @param listener to be notified.
         */
        void Schedule(const timeslice& target, EventListenerPtr listener);
        
        /**
         * Will create a temporal window from current ticket 
         * to (current tick + given ticks).
         * Each created window in this method will fire 2 events:
         *  - EM_WINDOW_UPDATE (only if the window is updateable)
         *  - EM_WINDOW_EXPIRED
         *
         * Attention: The listener will not be deleted by manager. 
         * @param target time to schedule.
         * @param listener to be notified.
         * @param callback to be called when the event is fired.
         */
        void Schedule(const timeslice& target, EventListenerPtr listener, 
                ListenerContextCallback callback);

    private:

        /**
         * Timeslice comparator.
         */
        struct TimesliceComparator {

            bool operator()(const timeslice& val1, const timeslice& val2) const {
                return (val1.ms() < val2.ms() || 
                        (val1.ms() == val2.ms() && val1.frame() < val2.frame()));
            }
        };

        /**
         * Internal class to represent a temporal window.
         * @param from, start time.
         * @param to, target time.
         */
        class TemporalWindow {
        public:
            TemporalWindow(const timeslice& from, const timeslice& to);
            virtual ~TemporalWindow();

            /**
             * Tells if window is expired on given timeslice.
             * @param current timeslice.
             * @return True if window is expired or false if not.
             */
            bool IsExpired(const timeslice& current) const;

            /**
             * Gets the initial timeslice.
             * @return initial timeslice. 
             */
            const timeslice& GetTo() const;

            /**
             * Gets the final timeslice.
             * @return final timeslice. 
             */
            const timeslice& GetFrom() const;
        private:
            timeslice from;
            timeslice to;
        };

        typedef list<TemporalWindow*> TemporalWindowList;
        typedef map<timeslice, TemporalWindowList*, TimesliceComparator> TemporalWindowMap;

    private:
        timeslice currTime;
        TemporalWindowMap temporalWindows;
        mutable shared_mutex windowsMutex;
    };
}

