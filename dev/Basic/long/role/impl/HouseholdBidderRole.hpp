//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdBidderRole.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 5:13 PM
 */
#pragma once
#include <boost/unordered_map.hpp>
#include "event/LT_EventArgs.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"
#include <util/TimeCheck.hpp>

namespace sim_mob
{
    namespace long_term
    {
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
        class HouseholdBidderRole
        {
        private:

            /**
             * Simple class to store the current unit which the bidder is trying to buy.
             */
            class CurrentBiddingEntry
            {
            public:
                CurrentBiddingEntry(const BigSerial unitId = INVALID_ID, const double bestBid = 0, const double wp = 0, double lastSurplus = 0, double wtp_e  = 0, double affordability = 0);
                ~CurrentBiddingEntry();

                /**
                 * Getters & setters 
                 */
                BigSerial getUnitId() const;
                double getWP() const;
                double getBestBid() const;
                void  setBestBid(double val);

                long int getTries() const;
                bool isValid() const;
                
                double getLastSurplus() const;
                void setLastSurplus(double value);

                double getWtp_e();
                void setWtp_e(double value);

                double getAffordability() const;
                void setAffordability( double value);

                /**
                 * Increments the tries variable with given quantity.
                 * @param quantity to increment.
                 */
                void incrementTries(int quantity = 1);
                void invalidate();
            private:
                BigSerial unitId;
                double wp; // willingness to pay.
                double bestBid; //actual final bid
                long int tries; // number of bids sent to the seller.
                double lastSurplus; // value of the last surplus
                double wtp_e; //willingToPay error term
                double affordability;
            };

        public:
            HouseholdBidderRole(HouseholdAgent* parent);
            virtual ~HouseholdBidderRole();

            bool isActive() const;
            void setActive(bool active);
            HouseholdAgent* getParent();

            void computeHouseholdAffordability();
            void computeBidValueLogistic( double price, double wp, double &finalBid, double &finalSurplus );

            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void update(timeslice currTime);

            /*
             * check all the vehicle categories and returns if it includes a motorcycle
             */
            bool isMotorCycle(int vehicleCategoryId);

            int getIncomeCategoryId(double income);

            int getMoveInWaitingTimeInDays();

            void setMoveInWaitingTimeInDays(int days);

            void setUnitIdToBeOwned(BigSerial unitId);

        protected:

            /**
             * Inherited from LT_Role
             */
            virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

        private:
            friend class HouseholdAgent;

            void init();

            /**
             * Helper method that goes to the market, gets the available units
             * and calculates the unit with maximum surplus. 
             * If the household as willingness to pay for a unit then makes a bit.  
             * @param now
             * @return 
             */
            bool bidUnit(timeslice now);

            /**
             *  After a bid has been successful, there is a waiting time for a household to take ownership of a unit.
             *  Once the count down, in days, is complete, the function is called.
             */
            void TakeUnitOwnership();

            /**
             * Picks a new market entry to bid.
             * Attention this function updates the value on biddingEntry variable.
             * @return true if a unit was picked false otherwise;
             */
            bool pickEntryToBid();
            double calculateWillingnessToPay(const Unit* unit, const Household* household, double& wtp_e);

            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            CurrentBiddingEntry biddingEntry;
            HouseholdAgent *parent;
            bool active;
        	BigSerial unitIdToBeOwned;
        	int moveInWaitingTimeInDays;
        	bool bidComplete;
        	int vehicleBuyingWaitingTimeInDays;
        	uint32_t day;
        	int year;

            enum EthnicityId
            {
            	CHINESE = 1, MALAY, INDIAN, OTHERS
            };

            enum CoeffParamId
            {
            	ASC_NO_CAR = 1, ASC_ONECAR, ASC_TWOplusCAR, B_ABOVE60_ONE_CAR, B_ABOVE60_TWOplusCAR,
            	B_INC1_ONECAR, B_INC1_TWOplusCAR, B_INC2_ONECAR, B_INC2_TWOplusCAR,B_INC3_ONECAR, B_INC3_TWOplusCAR, B_INC4_ONECAR, B_INC4_TWOplusCAR, B_INC5_ONECAR, B_INC5_TWOplusCAR,
            	B_INDIAN_ONECAR, B_INDIAN_TWOplusCAR, B_KIDS1_ONECAR, B_KIDS1_TWOplusCAR, B_KIDS2p_ONECAR, B_KIDS2p_TWOplusCAR, B_MALAY_ONECAR, B_MALAY_TWOplusCAR,B_MC_ONECAR,
            	B_MC_TWOplusCAR,  B_OTHERS_ONECAR, B_OTHERS_TWOplusCAR,B_PRIVATE_ONECAR, B_PRIVATE_TWOplusCAR, B_TAXI_ONECAR, B_TAXI_TWOplusCAR, B_WHITECOLLAR1_ONECAR, B_WHITECOLLAR1_TWOplusCAR, B_WHITECOLLAR2_ONECAR,
            	B_WHITECOLLAR2_TWOplusCAR, B_distMRT1000_ONECAR, B_distMRT1000_TWOplusCAR, B_distMRT500_ONECAR, B_distMRT500_TWOplusCAR,
            	B_LOGSUM_ONECAR,B_LOGSUM_TWOplusCAR
            };
            bool initBidderRole;
        };
    }
}
