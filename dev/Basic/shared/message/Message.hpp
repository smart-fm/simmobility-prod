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

    typedef int MessageType;
    
    #define MSG_CAST(_msg_type, _var_name) \
        static_cast<_msg_type*>(const_cast<Message*>(&_var_name))

    /**
     * Represents a message data that can be exchanged on Messaging Entities.
     */
    class Message {
    public:
        Message();
        Message(const Message& orig);
        virtual ~Message();
        Message& operator=(const Message& source);
    };
}

