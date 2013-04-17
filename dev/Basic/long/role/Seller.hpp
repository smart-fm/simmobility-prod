/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 5:13 PM
 */
#pragma once

#include "model/Bid.hpp"
#include "role/LT_Role.hpp"
#include "entity/HousingMarket.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Sellers role.
         * 
         * An agent with this role is selling units.
         *  
         */
        class Seller : public LT_Role {
        public:
            Seller(LT_Agent* parent, HousingMarket* market);
            virtual ~Seller();

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
            HousingMarket* market;
        };
    }
}