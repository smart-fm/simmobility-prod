//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventPublisher.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 7, 2013, 11:30 AM
 */

#pragma once

#include <list>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include "EventListener.hpp"

///Helper macro for callback pointer declaration. 
#define DECLARATION_CALLBACK_PTR(PtrName, ListenerType, ArgsType)\
void (ListenerType::*PtrName)(\
sim_mob::event::EventId id,\
sim_mob::event::Context ctxId,\
sim_mob::event::EventPublisher* sender,\
const ArgsType& args)

namespace sim_mob {

    namespace event {

        /**
         * Interface for Callbacks.
         */
        struct I_Callback {
        public:

            virtual ~I_Callback() {
            }

            virtual void operator()(
                    sim_mob::event::EventListener* listener,
                    sim_mob::event::EventId id,
                    sim_mob::event::Context ctxId,
                    sim_mob::event::EventPublisher* sender,
                    const sim_mob::event::EventArgs& args) = 0;
        };

        /**
         * Callback function template implementation.
         */
        template <typename L, typename T>
        struct CallbackImpl : public I_Callback {
            typedef DECLARATION_CALLBACK_PTR(CallbackRef, L, T);
            
        public:
            CallbackImpl(CallbackRef callback) 
                : callback(callback) {
            }

            virtual ~CallbackImpl() {
            }

            void operator()(
                    sim_mob::event::EventListener* listener,
                    sim_mob::event::EventId id,
                    sim_mob::event::Context ctxId,
                    sim_mob::event::EventPublisher* sender,
                    const sim_mob::event::EventArgs& args) {
                if (listener) {
                    (dynamic_cast<L*> (listener)->*callback)(id, ctxId, sender,
                            dynamic_cast<const T&> (args));
                }
            }
        private:
            CallbackRef callback;
        };

        typedef EventListener* EventListenerPtr;
        typedef boost::shared_ptr<I_Callback> Callback;

        /**
         * Struct to store a Listener entry.
         */
        typedef struct ListenerEntry {
            ListenerEntry(EventListenerPtr listener, Callback callback);
            EventListenerPtr listener;
            Callback callback;
        } Entry;

        // map for global listeners.
        typedef std::list<Entry> ListenersList;
        // maps for listeners with context.
        typedef boost::unordered_map<Context, ListenersList> ContextMap;
        typedef boost::unordered_map<EventId, ContextMap> ContextListenersMap;

        /**
         * Generic implementation of event publisher.
         * 
         * This implementation is not thread-safe. 
         */
        class EventPublisher {
        public:
            EventPublisher();
            virtual ~EventPublisher() = 0;

            /**
             * Registers a new event id on publisher.
             * @param id EventId to register.
             */
            virtual void registerEvent(EventId id);
            /**
             * UnRegisters the given event from the publisher. 
             * @param id EventId to un-register.
             */
            virtual void unRegisterEvent(EventId id);

            /**
             * Verifies if register exists.
             * @param id
             * @return 
             */
            virtual bool isEventRegistered(EventId id) const;

            /**
             * Publishes an event with given EventId.
             * @param id to publish.
             * @param args of the event.
             */
            virtual void publish(EventId id, const EventArgs& args);

            /**
             * Publishes an event with given EventId and ContextId.
             * @param id to publish.
             * @param ctxId Id of the context.
             * @param args of the event.
             */
            virtual void publish(EventId id, Context ctxId,
                    const EventArgs& args);

            /**
             * Subscribes the given listener to the given EventId.
             * Using the given callback and context.
             * 
             * @param id
             * @param listener to subscribe.
             * @param callback custom event handler.
             * @param context to filter the events.
             */
            template<typename L, typename T>
            void subscribe(EventId id,
                    L* listener,
                    DECLARATION_CALLBACK_PTR(callback, L, T),
                    Context context = 0) {
                if (callback) {
                    Callback cb(new CallbackImpl<L, T>(callback));
                    subscribe(id, listener, cb, context);
                } else {
                    subscribe(id, listener, context);
                }
            }

            /**
             * 
             * Subscribes the given global listener to the given EventId.
             * This listener will receive all events of the given EventId.
             * 
             * @param id
             * @param listener to subscribe.
             * @param context to filter events.
             */
            virtual void subscribe(EventId id, EventListenerPtr listener,
                    Context context = 0);

            /**
             * UnSubscribes the given listener to the given EventId
             * 
             * ATTENTION: the listener instance will not be deleted.
             * 
             * @param id of the event.
             * @param listener to UnSubscribe.
             */
            virtual void unSubscribe(EventId id, EventListenerPtr listener);

            /**
             * UnSubscribes the given listener to the given EventId and ContextId.
             * 
             * ATTENTION: the listener instance will not be deleted.
             * 
             * @param id of the event.
             * @param ctxId Id of the context.
             * @param listener to UnSubscribe.
             */
            virtual void unSubscribe(EventId id, Context ctxId,
                    EventListenerPtr listener);

            /**
             * UnSubscribes all global subscribers for the given event id. 
             * 
             * ATTENTION: the listener instances will not be deleted.
             *  
             * @param id of the global event.
             * 
             */
            void unSubscribeAll(EventId id);

            /**
             * UnSubscribes all context subscribers for the given event id and context.
             *
             * ATTENTION: the listener instances will not be deleted.
             * 
             * @param id of the event.
             * @param ctx context.
             */
            void unSubscribeAll(EventId id, Context ctx);

            /**
             * UnSubscribes the given listener from all events.
             * Attention: This method is not efficient 
             * is not fast you should avoid to call this method. 
             * @param listener to unsubscribe.
             */
            void unSubscribeAll(EventListenerPtr listener);

        protected:
            /**
             * Subscribes the given listener to the given EventId.
             * Using the given callback and context.
             * 
             * @param id
             * @param listener to subscribe.
             * @param callback custom event handler.
             * @param context to filter the events.
             */
            virtual void subscribe(EventId id, EventListenerPtr listener,
                    Callback callback, Context context = 0);

        private:
            ContextListenersMap listeners;
        };
    }
}
