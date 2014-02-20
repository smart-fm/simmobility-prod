//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   MessageReceiver.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Aug 15, 2013, 9:30 PM
 */
#pragma once
#include "MessageHandler.hpp"
#include "event/EventListener.hpp"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace sim_mob {

    namespace messaging {
        
        class MessageBus : public MessageHandler {
        public:
            typedef boost::shared_ptr<Message> MessagePtr;
            typedef boost::shared_ptr<event::EventArgs> EventArgsPtr;
            /**
             * Registers the main thread that will manage all MessageBus system.
             * Attention: You must call this function using the main thread.
             * @throws runtime_exception if the system has 
             *         already a context for the main thread.
             */
            static void RegisterMainThread();

            /**
             * Registers a new thread creating a context for it.
             * Attention: You must call this function using the thread context.
             * @throws runtime_exception if the system has 
             *         already a context for the current thread.
             */
            static void RegisterThread();

            /**
             * Registers a new handler using the context of the thread that is calling
             * the system.
             * @param handler to register.
             * @throws runtime_exception if the given the handler 
             *         has already a context associated or if the thread that calls 
             *         is not registered.
             */
            static void RegisterHandler(MessageHandler* handler);

            /**
             * UnRegisters the main thread.
             * Attention: You must call this function using the main thread.
             * @throws runtime_exception if the system has 
             *         already a context for the main thread.
             */
            static void UnRegisterMainThread();

            /**
             * UnRegisters the current thread.
             * Attention: You must call this function using the thread context.
             * @throws runtime_exception if the system has 
             *         already a context for the current thread.
             */
            static void UnRegisterThread();

            /**
             * UnRegisters given handler.
             * @param handler to unregister.
             * @throws runtime_exception if the given the handler 
             *         has already a context associated or if the thread that calls 
             *         is not registered.
             */
            static void UnRegisterHandler(MessageHandler* handler);

            /**
             * MessageBus distributes all messages for all registered threads.
             * Collects all messages from output queues of all thread contexts and
             * copies them to the output queue of the main message handler.
             * After that messages are distributed for the correspondent
             * thread input queue.
             * 
             * Note: All internal messages are processed before all custom messages.
             * Attention: This function should be called by the main thread.
             * 
             * @throws runtime_exception if the thread that calls is not the main thread.
             */
            static void DistributeMessages();

            /**
             * MessageBus distributes all messages for all registered threads.
             * Attention: This function should be called using each thread (context).
             * You don't need to call this function for the main thread.
             * @throws runtime_exception if the thread that calls has not any context associated.
             */
            static void ThreadDispatchMessages();

            /**
             * Posts a message on the current thread output queue.
             * The message will be posted& processed on the right queue
             * after the DistributeMessages() and ThreadDispatchMessages() calls.
             * @param target of the message.
             * @param type of the message.
             * @param message to send.
             * @param processOnMainThread processes the message on main thread
             * on a thread safe way. Attention if the value is true this message 
             * will be processed before the context thread messages. 
             */
            static void PostMessage(MessageHandler* target, Message::MessageType type, MessagePtr message, bool processOnMainThread = false);

            /**
             * Subscribes to the given event. 
             * This listener will receive *all* notifications for this event.
             * @param id of the event.
             * @param listener to handle the event.
             */
            static void SubscribeEvent(event::EventId id, event::EventListener* listener);

            /**
             * Subscribes to the given event and context. 
             * This listener will receive *only* notifications for 
             * this event published by the given context.
             * @param id of the event.
             * @param ctx of the event.
             * @param listener to handle the event.
             */
            static void SubscribeEvent(event::EventId id, event::Context ctx, event::EventListener* listener);
            
            /**
             * UnSubscribes the given listener from the given event. 
             * @param id of the event.
             * @param listener to unsubscribe.
             */
            static void UnSubscribeEvent(event::EventId id, event::EventListener* listener);

            /**
             * UnSubscribes the given listener from the given context and event. 
             * @param id of the event.
             * @param ctx of the event.
             * @param listener to unsubscribe.
             */
            static void UnSubscribeEvent(event::EventId id, event::Context ctx, event::EventListener* listener);
            
            /**
             * UnSubscribes the all listeners from given event.
             * A message will be posted on event publishers of all threads 
             * to remove the listeners.
             * Once the priority of this message is higher than the event message
             * It is guarantee that listeners will receive last events.
             * @param id of the event.
             */
            static void UnSubscribeAll(event::EventId id);

            /**
             * UnSubscribes the all listeners from given event and context.
             * A message will be posted on event publishers of all threads 
             * to remove the listeners.
             * Once the priority of this message is higher than the event message
             * It is guarantee that listeners will receive last events.
             * @param id of the event.
             * @param ctx to remove all listeners.
             */
            static void UnSubscribeAll(event::EventId id, event::Context ctx);
            
            /**
             * Publishes a global event.
             * @param id of the event.
             * @param args event data.
             */
            static void PublishEvent(event::EventId id, EventArgsPtr args);

            /**
             * Publishes an event only for the given context listeners.
             * @param id of the event.
             * @param ctx of the event.
             * @param args event data.
             */
            static void PublishEvent(event::EventId id, event::Context ctx, EventArgsPtr args);

        public:
            static const unsigned int MB_MIN_MSG_PRIORITY;
            static const unsigned int MB_MSG_START;

        private:

            /**
             * Constructor.
             */
            MessageBus();

            /**
             * Handles all internal messages. 
             * @param type of the message.
             * @param message to be processed internally.
             */
            void HandleMessage(Message::MessageType type, const Message& message);

            /**
             * Collects & dispatches messages from&to all thread contexts.
             * Attention: This function should be called by the main thread.
             */
            static void DispatchMessages();

            /**
             * Gets the main message bus instance.
             * @return MessageBus instance.
             */
            static MessageBus& GetInstance();
        };
        
        /**
         * Alias name for MessageBus
         */
        typedef MessageBus MsgBus;
    }
}
