/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HouseholdSellerRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once

#include "model/Bid.hpp"
#include "role/LT_Role.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {

    namespace long_term {
        
        class HouseholdAgent;
        
        /**
         * Household Seller role.
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
            friend class HouseholdAgent;
            HousingMarket* market;
            Household* hh;
        };
    }
}