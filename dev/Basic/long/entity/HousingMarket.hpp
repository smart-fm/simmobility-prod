/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HousingMarket.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:13 PM
 */
#pragma once

#include <map>
//#include "agent/Seller.hpp"
//#include "agent/Household.hpp"

//using std::map;
//using std::pair;
//using std::string;
//using std::vector;
//using namespace sim_mob;
//
//namespace sim_mob {
//
//    namespace long_term {
//        typedef pair<UnitId, Unit*> HM_UnitMapEntry;
//        typedef map<UnitId, Unit*> HM_UnitMap;
//
//        /**
//         * Represents the housing market.
//         * Th main responsibility is the management of: 
//         *  - bids
//         *  - sells
//         *  - avaliable units
//         *  - available households
//         * 
//         * The market is an updateable entity to simulate the evolution.
//         * 
//         */
//        class HousingMarket : public Entity{
//        public:
//            HousingMarket(unsigned int id = -1);
//            virtual ~HousingMarket();
//
//            /**
//             * Inherited from Entity.
//             */
//            virtual Entity::UpdateStatus update(timeslice now);
//
//            /**
//             * Registers a new unit to the market.
//             * @param unit to add.
//             */
//            void RegisterUnit(Unit* unit);
//
//            /**
//             * UnRegisters a unit from the market.
//             * @param unit to add.
//             */
//            void UnRegisterUnit(Unit* unit);
//
//            /**
//             * Verifies if the given unit is registered.
//             * @param unit to verify.
//             * @return true if exists, false otherwise.
//             */
//            bool IsUnitRegistered(Unit* unit) const;
//
//            /**
//             * Verifies if the given unit is available.
//             * @param unit to verify.
//             * @return true if exists, false otherwise.
//             */
//            bool IsUnitAvailable(Unit* unit) const;
//
//            /**
//             * An offer will be submitted.
//             */
//            void BidUnit(const Bid& bid);
//
//            /**
//             * Gets all available units.
//             * @return outUnits (out parameter) to receive the units.
//             */
//            void GetAvailableUnits(list<Unit*>& outUnits);
//
//        protected:
//
//            /**
//             * Inherited from Entity.
//             */
//            virtual void buildSubscriptionList(vector<BufferedBase*>& subsList);
//
//        private:
//            HM_UnitMap units;
//        };
//    }
//}
//
