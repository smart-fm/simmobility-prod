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
#include <iostream>
#include <fstream>
#include <boost/unordered_map.hpp>

namespace sim_mob
{
    namespace long_term
    {
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
        class LoggerAgent : public Entity
        {
        public:
            // STATIC for now this will change in the future with new output mechanisms
            enum LogFile
            {
                BIDS,
                EXPECTATIONS,
                STDOUT,
                PARCELS,
                UNITS,
                PROJECTS,
                HH_PC,
                UNITS_IN_MARKET,
                LOG_TAXI_AVAILABILITY,
                LOG_VEHICLE_OWNERSIP,
                LOG_TAZ_LEVEL_LOGSUM,
                LOG_HOUSEHOLDGROUPLOGSUM,
                LOG_INDIVIDUAL_HITS_LOGSUM,
                LOG_HOUSEHOLDBIDLIST,
                LOG_INDIVIDUAL_LOGSUM_VO,
				LOG_SCREENINGPROBABILITIES,
				LOG_HHCHOICESET,
				LOG_ERROR,
				LOG_SCHOOL_ASSIGNMENT,
				LOG_PRE_SCHOOL_ASSIGNMENT
            };

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
            void log(LogFile outputType, const std::string& logMsg);
            
        protected:
            /**
             * Inherited from Entity
             */
            virtual bool isNonspatial();
            virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);
            virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

        private:
            /**
             * Inherited from Entity
             */
            void onWorkerEnter();
            void onWorkerExit();

        private:
            typedef boost::unordered_map<LogFile, std::ofstream*> Files;
            boost::unordered_map<LogFile, std::ofstream*> streams; 
        };
    }
}
