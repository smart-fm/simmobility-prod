/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageReceiver.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Aug 15, 2013, 9:30 PM
 */
#pragma once
#include <queue>
#include "MessageHandler.hpp"
#include <boost/thread.hpp>

namespace sim_mob {

    namespace messaging {

        class MessageBus {
        public:
            static void RegisterMainCollector();
            static void RegisterThreadMessageQueue();
            static void RegisterHandler(MessageHandler* target);
            
            static void DistributeMessages();
            
            static void ThreadDispatchMessages();
            static void PostMessage(MessageHandler* target, Message::MessageType type, Message* message);
        private:
            static void CollectMessages();
            static void DispatchMessages();
            
        };
    }
}
