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
#include "database/entity/Bid.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;

        /**
         * Household Seller role.
         *
         * Seller will receive N bids each day and it will choose 
         * the maximum bid *of the time unit* (in this case is DAY) 
         * that satisfies the seller's asking price.
         */
        class HouseholdSellerRole : public LT_AgentRole<HouseholdAgent> {
        public:
            HouseholdSellerRole(HouseholdAgent* parent, Household* hh, 
                    HousingMarket* market);
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
            
            /**
             * Decides over a given bid for a given unit.
             * @param bid given by the bidder.
             * @return true if accepts the bid or false otherwise.
             */
            bool decide(const Bid& bid, const ExpectationEntry& entry);
            
            /**
             * Adjust the unit parameters for the next bids. 
             * @param unit
             */
            void adjustUnitParams(Unit& unit);
            
            /**
             * Notify the bidders that have their bid were accepted.
             */
            void notifyWinnerBidders();
            
            /**
             * Adjust parameters of all units that were not selled.
             */
            void adjustNotSelledUnits();
            
            /**
             * Calculates the unit expectations to the maximum period of time 
             * that the seller is expecting to be in the market.
             * @param unit to cal
             */
            void calculateUnitExpectations(const Unit& unit);
        
            /**
             * Gets current expectation entry for given unit.
             * @param unit to get the expectation.
             * @param outEntry (outParameter) to fill with the expectation. 
             *        If it not exists the values should be 0.
             * @return true if exists valid expectation, false otherwise.
             */
            bool getCurrentExpectation(const Unit& unit, ExpectationEntry& outEntry);
        private:

            typedef std::vector<ExpectationEntry> ExpectationList;
            typedef std::pair<UnitId, ExpectationList> ExpectationMapEntry; 
            typedef boost::unordered_map<UnitId, ExpectationList> ExpectationMap; 
            typedef boost::unordered_map<UnitId, Bid> Bids;
            typedef std::pair<UnitId, Bid> BidEntry;
            
            friend class HouseholdAgent;
            HousingMarket* market;
            Household* hh;
            timeslice currentTime;
            volatile bool hasUnitsToSale;
            //Current max bid information.
            Bids maxBidsOfDay;
            ExpectationMap unitExpectations;
            int currentExpectationIndex;
        };
    }
}
