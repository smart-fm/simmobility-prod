/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageHandler.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Aug 15, 2013, 9:30 PM
 */
#pragma once
#include "Message.hpp"

namespace sim_mob {

    namespace messaging {

        class MessageHandler {
        public:
            MessageHandler(int id);
            virtual ~MessageHandler();
            
            virtual void HandleMessage(Message::MessageType type, const Message& message) = 0;
            
            int GetId();
        protected: 
            int id;
        };
    }
}
