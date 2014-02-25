//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdSellerRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once

#include <boost/unordered_map.hpp>
#include "role/LT_Role.hpp"
#include "database/entity/Bid.hpp"
#include "database/entity/Household.hpp"
#include "database/entity/Unit.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;
        class HM_Model;
        class HousingMarket;
        /**
         * Household Seller role.
         *
         * Seller will receive N bids each day and it will choose 
         * the maximum bid *of the time unit* (in this case is DAY) 
         * that satisfies the seller's asking price.
         */
        class HouseholdSellerRole : public LT_AgentRole<HouseholdAgent> {
        public:
            HouseholdSellerRole(HouseholdAgent* parent);
            virtual ~HouseholdSellerRole();

            /**
             * Method that will update the seller on each tick.
             * @param currTime
             */
            virtual void update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(messaging::Message::MessageType type,
                const messaging::Message& message);

        private:
            friend class HouseholdAgent;
            /**
             * Notify the bidders that have their bid were accepted.
             */
            void notifyWinnerBidders();
            
            /**
             * Adjust parameters of all units that were not sold.
             */
            void adjustNotSoldUnits();
            
            /**
             * Calculates the unit expectations to the maximum period of time 
             * that the seller is expecting to be in the market.
             * @param unit to cal
             */
            void calculateUnitExpectations(const Unit& unit);
        
            /**
             * Gets current expectation entry for given unit.
             * @param unitId to get the expectation.
             * @param outEntry (outParameter) to fill with the expectation. 
             *        If it not exists the values should be 0.
             * @return true if exists valid expectation, false otherwise.
             */
            bool getCurrentExpectation(const BigSerial& unitId, ExpectationEntry& outEntry);
        private:
            typedef std::vector<ExpectationEntry> ExpectationList;

            struct SellingUnitInfo {
                SellingUnitInfo();
                ExpectationList expectations;
                unsigned int startedDay; //day when the unit was added on market.
                unsigned int daysOnMarket; //number of days to stay on market.
                unsigned int interval; //interval to re-evaluate the expectation.
                unsigned int numExpectations; //ceil(daysOnMarket/interval) to re-evaluate the expectation.
            };
            typedef boost::unordered_map<BigSerial, SellingUnitInfo> UnitsInfoMap;
            typedef boost::unordered_map<BigSerial, Bid> Bids;
            
            timeslice currentTime;
            volatile bool hasUnitsToSale;
            //Current max bid information.
            Bids maxBidsOfDay;
            UnitsInfoMap sellingUnitsMap;
            volatile bool selling;
        };
    }
}
