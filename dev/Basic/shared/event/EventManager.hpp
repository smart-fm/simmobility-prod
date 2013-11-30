//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   EventManager.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 1, 2013, 11:27 AM
 */
#pragma once

#include <boost/thread.hpp>

#include "EventPublisher.hpp"
#include "metrics/Frame.hpp"

/**
 * All external events must start with this id.
 */
//const int EM_EVENT_ID_START =10000;

namespace sim_mob {
    
    namespace event {
        // Events for HousingMarket
    	enum HouseMarketEvents {
    		EM_WND_EXPIRED = 0,
    		EM_WND_UPDATED = 1,
    	};


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
            void update(const timeslice& currTime);

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
            void schedule(const timeslice& target, EventListenerPtr listener);

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
            void schedule(const timeslice& target, EventListenerPtr listener,
                    Callback callback);

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
                bool isExpired(const timeslice& current) const;

                /**
                 * Gets the initial timeslice.
                 * @return initial timeslice. 
                 */
                const timeslice& getTo() const;

                /**
                 * Gets the final timeslice.
                 * @return final timeslice. 
                 */
                const timeslice& getFrom() const;
            private:
                timeslice from;
                timeslice to;
            };

            typedef std::list<TemporalWindow> TemporalWindowList;
            typedef std::map<timeslice, TemporalWindowList, TimesliceComparator> TemporalWindowMap;

        private:
            timeslice currTime;
            TemporalWindowMap temporalWindows;
            mutable boost::shared_mutex windowsMutex;
        };
    }
}

