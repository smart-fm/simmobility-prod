//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   ConcurrentEventPublisher.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on August 25, 2013, 11:30 AM
 */

#pragma once

#include <boost/thread.hpp>
#include "EventPublisher.hpp"

namespace sim_mob {

    namespace event {

        /**
         * Thread-Safe implementation of event publisher.
         */
        class ConcurrentEventPublisher : public EventPublisher {
        public:
            ConcurrentEventPublisher();
            virtual ~ConcurrentEventPublisher()= 0;

            /**
             * Inherited from event publisher.
             */
            virtual void registerEvent(EventId id);
            virtual void unRegisterEvent(EventId id);
            virtual bool isEventRegistered(EventId id) const;
            virtual void publish(EventId id, const EventArgs& args);
            virtual void publish(EventId id, Context ctxId, const EventArgs& args);
            virtual void unSubscribe(EventId id, EventListenerPtr listener);
            virtual void unSubscribe(EventId id, Context ctxId, EventListenerPtr listener);
            virtual void unSubscribeAll(EventId id);
            virtual void unSubscribeAll(EventId id, Context ctx);
            
        protected:
            virtual void subscribe(EventId id, EventListenerPtr listener, 
                    Callback callback, Context context = 0);
        private:
            mutable boost::shared_mutex listenersMutex;
        };
    }
}
