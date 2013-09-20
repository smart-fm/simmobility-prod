//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Message.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 8, 2013, 6:16 PM
 */
#pragma once

namespace sim_mob {
    namespace messaging {
        /**
         * Helper Macro
         * Cast given constant Message instance reference (_msg_ref)
         * to a constant <msg_type>* pointer.
         * 
         * Example:
         * 
         * void HandleMessage(Message::Type type, MessageReceiver& sender, 
         *               const Message& message){
         *  if (type == MY_MESSAGE_TYPE){
         *      const MyMessage* concreteMessagePtr = MSG_CAST_PTR(MyMessage, msg);
         *  }
         * }
         */
#define MSG_CAST_PTR(_msg_type, _msg_ref) \
        static_cast<const _msg_type*>(&_msg_ref)

        /**
         * Helper Macro
         * Cast given constant Message instance reference (_msg_ref)
         * to a constant <msg_type>& reference.
         * 
         * Example:
         * void HandleMessage(Message::Type type, MessageReceiver& sender, 
         *               const Message& message){
         *  if (type == MY_MESSAGE_TYPE){
         *      const MyMessage& concreteMessage = MSG_CAST(MyMessage, msg);
         *  }
         * }
         */
#define MSG_CAST(_msg_type, _msg_ref) \
        static_cast<const _msg_type&>(_msg_ref)
        
        /**
         * Represents a message data that can be exchanged on Messaging Entities.
         */
        class MessageHandler;
        class Message {
        public:
            
            typedef int MessageType;
            Message();
            Message(const Message& orig);
            virtual ~Message();
            Message& operator=(const Message& source);
            int GetPriority() const;
            MessageHandler* GetSender() const;
            void SetSender(MessageHandler* sender);
        protected:
            friend class MessageBus;
            int priority;
            MessageHandler* sender;
        };
    }
}

