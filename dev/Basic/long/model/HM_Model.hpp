/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 21, 2013, 3:08 PM
 */
#pragma once
#include "Model.hpp"
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>
#include "database/entity/Household.hpp"
#include "database/entity/Unit.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Class that contains Housing market model logic.
         */
        class HM_Model : public Model{
        public:
            HM_Model(db::DB_Config& dbConfig, WorkGroup& workGroup);
            virtual ~HM_Model();
            const Unit* getUnitById(const BigSerial& unitId) const;
        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();
        
        private:
            typedef boost::unordered_map<BigSerial, Unit*> UnitMap;      
            // Data
            HousingMarket market;
            std::vector<Household> households;
            std::vector<Unit> units;
            UnitMap unitsById;
        };
    }
}

