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
#include "database/entity/ExternalEvent.hpp"
#include "database/entity/PostcodeAmenities.hpp"
#include "database/entity/PotentialProject.hpp"
#include "core/HousingMarket.hpp"
#include "model/HM_Model.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Represents the Lua model to for external events injection.
         */
        class ExternalEventsModel : public lua::LuaModel {
        public:
            ExternalEventsModel();
            ExternalEventsModel(const ExternalEventsModel& orig);
            virtual ~ExternalEventsModel();

            /**
             * Gets all external events.
             *
             * @param day.
             * @param outValues receive all external events.
             */
            void getExternalEvents(int day,
                    std::vector<ExternalEvent>& outValues) const;
        private:

            /**
             * Inherited from LuaModel
             */
            void mapClasses();
        };

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
             * Calculates the surplus for the given market entry.
             * 
             * @param entry target.
             * @param unitBids number of bids (attempts) to the given entry.
             * @return value of the Surplus or 
             *         sim_mob::long_term::INVALID_DOUBLE
             */
            double calculateSurplus(const HousingMarket::Entry& entry, int unitBids) const;

            /**
             * Calculates the willingness to pay based on Household attributes 
             * (and importance) and unit attributes.
             * 
             * @param household.
             * @param unit to calculate the wp.
             * @param stats Taz statistics.
             * @return value of the willingness to pay of the given household 
             *         or sim_mob::long_term::INVALID_DOUBLE
             */
            double calulateWP(const Household& hh, const Unit& unit, 
                const HM_Model::TazStats& stats) const;

        private:

            /**
             * Inherited from LuaModel
             */
            void mapClasses();
        };

        /**
         * Represents the Lua model to the Developer model.
         */
        class DeveloperLuaModel : public lua::LuaModel {
        public:
            DeveloperLuaModel();
            virtual ~DeveloperLuaModel();

            /**
             * Calculates the future revenue with given potential unit for the 
             * given Potential project.
             * 
             * Attention: given postcode can be the real postcode (if it is a redevelopment)
             * or it can be the nearest postcode of the parcel.
             *  
             * @param unit to calculate the the future revenue.
             * @param postcode amenities.
             * @return value of the future revenue or long_term::INVALID_DOUBLE
             */
            double calulateUnitRevenue (const PotentialUnit& unit, 
                const PostcodeAmenities& amenities) const;
            
        private:

            /**
             * Inherited from LuaModel
             */
            void mapClasses();
        };
    }
}
