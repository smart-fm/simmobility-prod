/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LoggerAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 21, 2014, 1:32 PM
 */
#pragma once

#include "entities/Entity.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Entity responsible log messages in a thread-safe way without logs.
         * 
         * This special agent uses the MessageBus to receive messages to log. 
         * 
         * The output is depending on the given configuration.
         * 
         * Attention: This agent is designed to work with SimMobility Workers and 
         * MessageBus system. Do not use it within external threads.
         * 
         */
        class LoggerAgent : public Entity {
        public:
            LoggerAgent();
            virtual ~LoggerAgent();
            
            /**
             * Inherited from Entity
             */
            virtual UpdateStatus update(timeslice now);
            
            /**
             * Log the given message. 
             * @param logMsg to print.
             */
            void log(const std::string& logMsg);
            
        protected:
            /**
             * Inherited from Entity
             */
            virtual bool isNonspatial();
            virtual void buildSubscriptionList(
                std::vector<BufferedBase*>& subsList);
            virtual void HandleMessage(messaging::Message::MessageType type,
                    const messaging::Message& message);

        private:
            /**
             * Inherited from Entity
             */
            void onWorkerEnter();
            void onWorkerExit();
        };
    }
}