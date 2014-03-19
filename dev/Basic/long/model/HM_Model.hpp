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
            
            /**
             * Taz statistics
             */
            class TazStats {
            public:
                TazStats(BigSerial tazId = INVALID_ID);
                virtual ~TazStats();
                
                /**
                 * Getters 
                 */
                BigSerial getTazId() const;
                long int getHH_Num() const;
                double getHH_TotalIncome() const;
                double getHH_AvgIncome() const;
            private:
                friend class HM_Model;
                void updateStats(const Household& household);
            private:
                BigSerial tazId;
                long int hhNum;
                double hhTotalIncome;
            };
            
            typedef boost::unordered_map<BigSerial, HM_Model::TazStats*> StatsMap;
            
        public:
            HM_Model(WorkGroup& workGroup);
            virtual ~HM_Model();
            
            /**
             * Getters & Setters 
             */
            const Unit* getUnitById(BigSerial id) const;
            BigSerial getUnitTazId(BigSerial unitId) const;
            const TazStats* getTazStats(BigSerial tazId) const;
            const TazStats* getTazStatsByUnitId(BigSerial unitId) const;
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
            StatsMap stats;
        };
    }
}

