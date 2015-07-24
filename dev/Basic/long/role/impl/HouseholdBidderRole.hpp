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
#include "event/LT_EventArgs.hpp"
#include "database/entity/Household.hpp"
#include "core/HousingMarket.hpp"

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
             * Simple struct to store the current unit which the bidder is trying to buy.
             */
            class CurrentBiddingEntry
            {
            public:
                CurrentBiddingEntry(const BigSerial unitId = INVALID_ID, const double wp = 0, double lastSurplus = 0 );
                ~CurrentBiddingEntry();

                /**
                 * Getters & setters 
                 */
                BigSerial getUnitId() const;
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
                BigSerial unitId;
                double wp; // willingness to pay.
                long int tries; // number of bids sent to the seller.
                double lastSurplus; // value of the last surplus
            };
        public:
            HouseholdBidderRole(HouseholdAgent* parent);
            virtual ~HouseholdBidderRole();

            bool isActive() const;
            void setActive(bool active);
            HouseholdAgent* getParent();

            void ComputeHouseholdAffordability();
            double ComputeBidValue( BigSerial householdId, BigSerial unitId, double ap, double wp );
            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void update(timeslice currTime);

            void reconsiderVehicleOwnershipOption();

            double getExpOneCar(int unitTypeId, double vehicleOwnershipLogsum);

            double getExpTwoPlusCar(int unitTypeId, double vehicleOwnershipLogsum);

            /*
             * check all the vehicle categories and returns if it includes a motorcycle
             */
            bool isMotorCycle(int vehicleCategoryId);

            int getIncomeCategoryId(double income);

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
            double calculateWillingnessToPay(const Unit* unit, const Household* household);
            double calculateSurplus(double price, double min, double max );

            volatile bool waitingForResponse;
            timeslice lastTime;
            bool bidOnCurrentDay;
            CurrentBiddingEntry biddingEntry;
            HouseholdAgent *parent;
            bool active;
        	BigSerial unitIdToBeOwned;
        	int moveInWaitingTimeInDays;
        	int vehicleBuyingWaitingTimeInDays;
        	uint32_t day;

            enum EthnicityId{
            	CHINESE = 1, MALAY, INDIAN, OTHERS
            };
            enum CoeffParamId{
            	ASC_NO_CAR = 1, ASC_ONECAR, ASC_TWOplusCAR, B_ABOVE60_ONE_CAR, B_ABOVE60_TWOplusCAR, B_CEO_ONECAR, B_CEO_TWOplusCAR,
            	B_FULLWORKER1_ONECAR, B_FULLWORKER1_TWOplusCAR, B_FULLWORKER2_ONECAR, B_FULLWORKER2_TWOplusCAR, B_FULLWORKER3p_ONECAR, B_FULLWORKER3p_TWOplusCAR, B_HAS_MC_ONECAR,
            	B_HAS_MC_TWOplusCAR, B_HHSIZE3_ONECAR, B_HHSIZE3_TWOplusCAR, B_HHSIZE4_ONECAR, B_HHSIZE4_TWOplusCAR, B_HHSIZE5_ONECAR, B_HHSIZE5_TWOplusCAR,
            	B_HHSIZE6_ONECAR, B_HHSIZE6_TWOplusCAR, B_INC12_ONECAR, B_INC12_TWOplusCAR, B_INC3_ONECAR, B_INC3_TWOplusCAR, B_INC4_ONECAR, B_INC4_TWOplusCAR, B_INC5_ONECAR, B_INC5_TWOplusCAR,
            	B_INC6_ONECAR, B_INC6_TWOplusCAR, B_INDIAN_ONECAR, B_INDIAN_TWOplusCAR, B_KID1_ONECAR, B_KID1_TWOplusCAR, B_KID2p_ONECAR, B_KID2p_TWOplusCAR, B_LANDED_ONECAR, B_LANDED_TWOplusCAR, B_LOGSUM_ONECAR,
            	B_LOGSUM_TWOplusCAR, B_MALAY_ONECAR, B_MALAY_TWOplusCAR, B_OTHER_RACE_ONECAR, B_OTHER_RACE_TWOplusCAR, B_PRIVATE_ONECAR, B_PRIVATE_TWOplusCAR, B_SELFEMPLOYED_ONECAR, B_SELFEMPLOYED_TWOplusCAR,
            	B_STUDENT1_ONECAR, B_STUDENT1_TWOplusCAR, B_STUDENT2_ONECAR, B_STUDENT2_TWOplusCAR, B_STUDENT3_ONECAR, B_STUDENT3_TWOplusCAR, B_WHITECOLLAR1_ONECAR, B_WHITECOLLAR1_TWOplusCAR, B_WHITECOLLAR2_ONECAR,
            	B_WHITECOLLAR2_TWOplusCAR, B_distMRT1000_ONECAR, B_distMRT1000_TWOplusCAR, B_distMRT500_ONECAR, B_distMRT500_TWOplusCAR
            };
            float householdAffordabilityAmount;
            bool initBidderRole;
        };
    }
}
