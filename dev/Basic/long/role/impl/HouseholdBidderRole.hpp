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
         * with maximum surplus for the bidder. After that the bidder 
         * will bid the unit and will *WAIT* for the response.
         * 
         * The bidder can only do a bid each day. 
         * If he is waiting for a response he will 
         * only able to do the next bid on the next day.
         */
        class HouseholdBidderRole : public LT_AgentRole<HouseholdAgent> {
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
             * Gets the bids counter for the given unit.
             * @param unitId unit unique identifier.
             * @return number of bids.
             */
            int getBidsCounter(const BigSerial& unitId);

            /**
             * Increments the bids counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void incrementBidsCounter(const BigSerial& unitId);

            /**
             * Deletes the counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void deleteBidsCounter(const BigSerial& unitId);

            /**
             * Picks a new market entry to bid.
             * @return HousingMarket::Entry const pointer or nullptr. 
             */
            const HousingMarket::Entry* pickEntryToBid() const;

        private:
            const HousingMarket::Entry* biddingEntry;
            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            typedef boost::unordered_map<BigSerial, int> BidsCounterMap; // bids made per unit.  
            typedef std::pair<BigSerial, int> BidCounterEntry;
            BidsCounterMap bidsPerUnit;
        };
    }
}