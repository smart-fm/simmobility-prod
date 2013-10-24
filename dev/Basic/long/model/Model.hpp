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

        class Model {
        public:
            Model(const std::string& name, db::DatabaseConfig& dbConfig, WorkGroup& workGroup);
            virtual ~Model();
            void start();
            void stop();
            bool isRunning() const;
            double getStartTime() const;
            double getStopTime() const;
            const std::string& getName() const;
        protected:
            virtual void startImpl() = 0;
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

