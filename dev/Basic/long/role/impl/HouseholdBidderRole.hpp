/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
#include "database/entity/housing-market/BidderParams.hpp"

namespace sim_mob {

    namespace long_term {

        class HouseholdAgent;

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
        class HouseholdBidderRole : public LT_AgentRole<HouseholdAgent>,
        public MessageReceiver {
        public:
            HouseholdBidderRole(HouseholdAgent* parent, Household* hh, 
                    const BidderParams& params, HousingMarket* market);
            virtual ~HouseholdBidderRole();

            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void Update(timeslice currTime);
        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(Message::Type type,
                    MessageReceiver& sender, const Message& message);
        private:
            /**
             * Handler for wakeup event.
             * @param id of the event.
             * @param ctx context of the event.
             * @param sender EVentManager responsible for the fired event.
             * @param args {@link EM_EventArgs} instance.
             */
            void OnWakeUp(EventId id, Context ctx,
                    EventPublisher* sender, const EM_EventArgs& args);

            /**
             * Handler for Market action event.
             * @param id of the event.
             * @param sender of the event.
             * @param args of the event.
             */
            void OnMarketAction(EventId id, EventPublisher* sender,
                    const HM_ActionEventArgs& args);

            /**
             * Subscribes the role to all market generic events.
             */
            void FollowMarket();

            /**
             * UnSubscribes the role to all market generic events.
             */
            void UnFollowMarket();

        private:
            friend class HouseholdAgent;
            
            /**
             * Helper method that goes to the market, gets the available units
             * and calculates the unit with maximum surplus. 
             * If the household as willingness to pay for a unit then makes a bit.  
             * @param now
             * @return 
             */
            bool BidUnit(timeslice now);
            
            /**
             * Calculates the surplus for the given unit.
             * 
             * surplus = pow(askingPrice, alpha + 1)/ (n_bids * zeta)
             * 
             * Where:
             *    n_bids: Represents the number of attempts(bids) that the bidder already did to the specific unit. 
             *    alpha: Represents the urgency of the household to get the unit. (Household parameter)
             *    zeta: Represents the relation between quality and price of the unit. (Unit parameter)
             * 
             * @param unit to calculate the surplus.
             * @return the surplus for the given unit.
             */
            float CalculateSurplus(const Unit& unit);

            /**
             * Calculates the willingness to pay based on Household 
             * attributes (and importance) and unit attributes.
             * 
             * This method calculates the willingness to pay following this formula:
             * 
             * wp = (HHIncomeWeight * HHIncome) + 
             *      (HHUnitAttributeWeight1 * UnitAttribute1) + ...
             *      (HHUnitAttributeWeightN * UnitAttributeN)
             *
             * @return value of the willingness to pay
             */
            float CalculateWP(const Unit& unit);

            /**
             * Gets the bids counter for the given unit.
             * @param unitId unit unique identifier.
             * @return number of bids.
             */
            int GetBidsCounter(UnitId unitId);

            /**
             * Increments the bids counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void IncrementBidsCounter(UnitId unitId);
            
            /**
             * Deletes the counter for the given unit.
             * @param unitId unit unique identifier.
             */
            void DeleteBidsCounter(UnitId unitId);

        private:
            Household* hh;
            HousingMarket* market;
            BidderParams params;
            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            typedef boost::unordered_map<UnitId, int> BidsCounterMap; // bids made per unit.  
            typedef std::pair<UnitId, int> BidCounterEntry;
            BidsCounterMap bidsPerUnit;
        };
    }
}

