/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 21, 2013, 3:08 PM
 */
#pragma once
#include <vector>
#include "workers/WorkGroup.hpp"
#include "database/DatabaseConfig.hpp"
#include "database/entity/Household.hpp"
#include "database/entity/Unit.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {
    namespace long_term {

        class HM_Model {
        public:
            HM_Model(db::DatabaseConfig& dbConfig, WorkGroup& workGroup);
            virtual ~HM_Model();
            void Start();
            void Stop();
        private:
            WorkGroup& workGroup;
            db::DatabaseConfig& dbConfig;
            bool started;
            // Data
            HousingMarket market;
            std::vector<Household> households;
            std::vector<Unit> units;
            std::vector<Agent*> agents;
        };
    }
}

