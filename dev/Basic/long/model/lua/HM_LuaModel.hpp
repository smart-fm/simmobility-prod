/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HMLuaModel.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 10, 2013, 2:39 PM
 */
#pragma once
#include <vector>
#include "Types.hpp"
#include "lua/LuaModel.hpp"
#include "database/entity/Unit.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Represents the Lua model to the Housing market model.
         */
        class HM_LuaModel : public lua::LuaModel {
        public:
            HM_LuaModel();
            HM_LuaModel(const HM_LuaModel& orig);
            virtual ~HM_LuaModel();

            /**
             * Calculates unit expectations and return them into given 
             * outValues.
             * 
             * @param unit target.
             * @param timeOnMarket number of expectations which are necessary 
             *        to calculate.
             * @param outValues vector that will hold returned expectations.
             */
            void calulateUnitExpectations(const Unit& unit, int timeOnMarket,
                    std::vector<ExpectationEntry>& outValues) const;

            /**
             * Calculates the hedonic price for the given unit.
             * 
             * @param unit target.
             * @return value of the Hedonic price or 
             *         sim_mob::long_term::INVALID_DOUBLE
             */
            double calculateHedonicPrice(const Unit& unit) const;

            /**
             * Calculates the surplus for the given unit.
             * 
             * @param entry target.
             * @param unitId target.
             * @param unitBids number of bids (attempts) to this unit.
             * @return value of the Surplus or 
             *         sim_mob::long_term::INVALID_DOUBLE
             */
            double calculateSurplus(const HousingMarket::Entry& entry, const Unit& unit, int unitBids) const;

            /**
             * Calculates the willingness to pay based on Household attributes 
             * (and importance) and unit attributes.
             * 
             * @param household.
             * @param unit to calculate the wp.
             * @return value of the willingness to pay of the given household 
             *         or sim_mob::long_term::INVALID_DOUBLE
             */
            double calulateWP(const Household& hh, const Unit& unit) const;
        private:

            /**
             * Inherited from LuaModel
             */
            void mapClasses();
        };
    }
}

