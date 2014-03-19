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
#include "database/entity/Household.hpp"
#include "database/entity/Unit.hpp"
#include "core/HousingMarket.hpp"
#include "boost/unordered_map.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Class that contains Housing market model logic.
         */
        class HM_Model : public Model {
        public:
            typedef std::vector<Unit*> UnitList;
            typedef std::vector<Household*> HouseholdList;
            typedef boost::unordered_map<BigSerial, Household*> HouseholdMap;
            typedef boost::unordered_map<BigSerial, Unit*> UnitMap;
        public:
            HM_Model(WorkGroup& workGroup);
            virtual ~HM_Model();
            
            /**
             * Getters & Setters 
             */
            const Unit* getUnitById(BigSerial id) const;
            BigSerial getUnitTazId(BigSerial unitId) const;
        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();

        private:
            // Data
            HousingMarket market;
            HouseholdList households;
            UnitList units; //residential only.
            HouseholdMap householdsById;
            UnitMap unitsById;
        };
    }
}

