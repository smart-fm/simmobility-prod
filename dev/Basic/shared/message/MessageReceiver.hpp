/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageReceiver.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 6, 2013, 3:30 PM
 */
#pragma once
#include <queue>
#include "Message.hpp"
#include <boost/thread.hpp>

namespace sim_mob {

    /**
     * Abstract implementation for all 
     * components that are capable to exchange messages.
     */
    class MessageReceiver {
    public:
        MessageReceiver();
        virtual ~MessageReceiver();

        /**
         * Read a message from queue and consume it.
         * @return true if any message was consumed, false otherwise.
         */
        virtual bool ReadMessage();

        /**
         * Posts an asynchronous message on receiver. 
         * This message will be putted on Receiver's queue.
         * 
         * Attention: The given {@link Message} will be managed by this component.
         * The message will be deleted after being consumed.
         * The given {@link MessageReceiver} will *NOT* be managed 
         * by the component.
         * @param type that identifies the type of the message.
         * @param sender of the message.
         * @param message data.
         */
        virtual void Post(Message::Type type, MessageReceiver* sender,
                Message* message);

        /**
         * Posts a synchronous message to the receiver. 
         * The message will be consumed on this call and thread context.
         * 
         * @param type that identifies the type of the message.
         * @param sender of the message.
         * @param message data.
         */
        virtual bool Send(Message::Type type, MessageReceiver& sender,
                const Message& message);
    protected:
        /**
         * Method to handle the given message.
         * @param type of the message.
         * @param sender {@link MessageReceiver} that have sent the message.
         * @param message data.
         */
        virtual void HandleMessage(Message::Type type, MessageReceiver& sender,
                const Message& message) = 0;
    private:
        //Definitions
        typedef std::pair<MessageReceiver*, Message*> MessageData;
        typedef std::pair<Message::Type, MessageData*> MessageEntry;
        typedef std::queue<MessageEntry*> MessageList;
        
        /**
         * Tells if exists any message to consume.
         * @return true if exists any message, false otherwise.
         */
        bool HasMessages();

        /**
         * Helper method.
         */
        bool SendMessage(Message::Type type, MessageReceiver* sender,
                Message* message, bool async);
        
        /**
         * Deletes a message entry.
         * @param entry to delete.
         */
        void DeleteEntry(MessageEntry* entry);
        
        /**
         * Verifies if exists messages or not.
         * @return true if exists any message on he queue or not.
         */
        bool ContainsMessages();
    private:
        MessageList messages;
        mutable boost::shared_mutex queueMutex;
    };
}
