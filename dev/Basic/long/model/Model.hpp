/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 21, 2013, 4:24 PM
 */
#pragma once
#include <vector>
#include <string>
#include "workers/WorkGroup.hpp"
#include "entities/Agent.hpp"
#include "database/DatabaseConfig.hpp"
#include "util/Utils.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Represents a generic model that runs within a given workgroup.
         */
        class Model {
        public:
            
            Model(const std::string& name, db::DatabaseConfig& dbConfig, 
                  WorkGroup& workGroup);
            virtual ~Model();
            
            /**
             * Starts the model. 
             * This method calls the abstract method startImpl.
             */
            void start();
            
            /**
             * Stops the model if it is running. 
             * This method calls the abstract method stopImpl.
             */
            void stop();
            
            /**
             * Tells if the model is running or not.
             * @return true is start method was called with success,
             *         false otherwise
             */
            bool isRunning() const;
             
             /**
              * Gets the time that the start method took to finish.
              * @return time.
              */
            double getStartTime() const;
            
            /**
             * Gets the time that the stop method took to finish.
             * @return 
             */
            double getStopTime() const;
            
            /**
             * Gets the module name.
             * @return module name.
             */
            const std::string& getName() const;
            
        protected:
            /**
             * Abstract method to be implemented by Model specializations.
             * You should load data from data sources, create agents
             * and fulfill the workgroup inside this method.  
             */
            virtual void startImpl() = 0;
            
            /**
             * Abstract method to be implemented by Model specializations to 
             * finalize the workgroup. 
             * 
             * NOTE: do not DELETE the agents inside this method.
             * Agents are deleted automatically in the destructor.
             */
            virtual void stopImpl() = 0;
        
        protected:
            db::DatabaseConfig& dbConfig;
            WorkGroup& workGroup;
            std::vector<Agent*> agents;
        
        private:
            volatile bool running;
            //watches
            StopWatch startWatch;
            StopWatch stopWatch;
            std::string name;
        };
    }
}

