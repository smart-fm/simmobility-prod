/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
            virtual void RegisterEvent(EventId id);
            virtual void UnRegisterEvent(EventId id);
            virtual bool IsEventRegistered(EventId id) const;
            virtual void Publish(EventId id, const EventArgs& args);
            virtual void Publish(EventId id, Context ctxId, const EventArgs& args);
            virtual void Subscribe(EventId id, EventListenerPtr listener);
            virtual void Subscribe(EventId id, EventListenerPtr listener, ListenerCallback eventFunction);
            virtual void Subscribe(EventId id, Context ctxId, EventListenerPtr listener);
            virtual void Subscribe(EventId id, Context ctxId, EventListenerPtr listener, ListenerContextCallback eventFunction);
            virtual void UnSubscribe(EventId id, EventListenerPtr listener);
            virtual void UnSubscribe(EventId id, Context ctxId, EventListenerPtr listener);
            virtual void UnSubscribeAll(EventId id);
            virtual void UnSubscribeAll(EventId id, Context ctx);
        private:
            mutable boost::shared_mutex listenersMutex;
        };
    }
}
