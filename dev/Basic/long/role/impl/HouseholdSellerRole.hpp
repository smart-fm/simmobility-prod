/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
            virtual void Update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(MessageType type,
                    MessageReceiver& sender, const Message& message);

            /**
             * Decides over a given bid for a given unit.
             * @param bid given by the bidder.
             * @return true if accepts the bid or false otherwise.
             */
            virtual bool Decide(const Bid& bid, const Unit& unit);

            /**
             * Adjust the unit parameters for the next bids. 
             * @param unit
             */
            virtual void AdjustUnitParams(Unit& unit);
            
        private:
            
            /**
             * Notify the bidders that have their bid were accepted.
             */
            void NotifyWinnerBidders();
            
            /**
             * Adjust parameters of all units that were not selled.
             */
            void AdjustNotSelledUnits();
            
            /**
             * Calculates the unit expectations to the maximum period of time 
             * that the seller is expecting to be in the market.
             * @param unit to cal
             */
            void CalculateUnitExpectations(const Unit& unit);
        
        private:
            typedef list<float> ExpectationEntry;
            typedef boost::unordered_map<UnitId, Bid> Bids; 
            typedef boost::unordered_map<UnitId, ExpectationEntry> Expectations; 
            typedef pair<UnitId, Bid> BidEntry;
            friend class HouseholdAgent;
            HousingMarket* market;
            Household* hh;
            timeslice currentTime;
            volatile bool hasUnitsToSale;
            //Current max bid information.
            Bids maxBidsOfDay;
            Expectations unitExpectations;
        };
    }
}