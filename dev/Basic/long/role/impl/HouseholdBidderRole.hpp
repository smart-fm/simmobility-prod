//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once
#include <boost/unordered_map.hpp>
#include "role/LT_Role.hpp"
#include "event/LT_EventArgs.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;
        class HM_Model;

        /**
         * Bidder role for household.
         * 
         * This role starts to go to the market and choose the unit 
         * with maximum surplus (WP - asking price) for the bidder. 
         * After that the bidder will bid the unit and will wait for the response.
         * 
         * The bidder can only do one bid each day and It sticks to the unit until
         * gets rejected or reaches the a zero surplus.
         */
        class HouseholdBidderRole : public LT_AgentRole<HouseholdAgent> {
        private:

            /**
             * Simple struct to store the current unit which the bidder is trying to buy.
             */
            class CurrentBiddingEntry {
            public:
                CurrentBiddingEntry(const HousingMarket::Entry* biddingEntry = nullptr, const double wp = 0);
                ~CurrentBiddingEntry();

                /**
                 * Getters & setters 
                 */
                const HousingMarket::Entry* getEntry() const;
                double getWP() const;
                long int getTries() const;
                bool isValid() const;
                
                /**
                 * Increments the tries variable with given quantity.
                 * @param quantity to increment.
                 */
                void incrementTries(int quantity = 1);
                void invalidate();
            private:
                const HousingMarket::Entry* entry;
                double wp; // willingness to pay.
                long int tries; // number of bids sent to the seller.
                double lastSurplus; // value of the last surplus
            };
        public:
            HouseholdBidderRole(HouseholdAgent* parent);
            virtual ~HouseholdBidderRole();

            /**
             * Inherited from LT_Role
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
             * Helper method that goes to the market, gets the available units
             * and calculates the unit with maximum surplus. 
             * If the household as willingness to pay for a unit then makes a bit.  
             * @param now
             * @return 
             */
            bool bidUnit(timeslice now);

            /**
             * Picks a new market entry to bid.
             * Attention this function updates the value on biddingEntry variable.
             * @return true if a unit was picked false otherwise;
             */
            bool pickEntryToBid();

        private:
            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            CurrentBiddingEntry biddingEntry;
        };
    }
}