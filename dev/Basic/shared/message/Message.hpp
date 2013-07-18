/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Message.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 8, 2013, 6:16 PM
 */
#pragma once

namespace sim_mob {

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
    class Message {
    public:
        typedef int Type;
        Message();
        Message(const Message& orig);
        virtual ~Message();
        Message& operator=(const Message& source);
    };
}

