/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageReceiver.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Aug 15, 2013, 9:30 PM
 */
#pragma once
#include "MessageHandler.hpp"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace sim_mob {

    namespace messaging {

        class MessageBus : MessageHandler {
        public:
            typedef boost::shared_ptr<Message> MessagePtr;
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
             * @param target of the message.
             * @param type of the message.
             * @param message to send.
             */
            static void PostMessage(MessageHandler* target, Message::MessageType type, MessagePtr message);

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
             * Collects messages from all thread contexts.
             * Attention: This function should be called by the main thread.
             */
            static void CollectMessages();

            /**
             * Dispatches messages to all thread contexts.
             * Attention: This function should be called by the main thread.
             */
            static void DispatchMessages();

            /**
             * Gets the main message bus instance.
             * @return MessageBus instance.
             */
            static MessageBus& GetInstance();
        };
    }
}
