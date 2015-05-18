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
            /**
             * Inherited from LT_Role
             * @param currTime
             */
            virtual void update(timeslice currTime);

            void reconsiderVehicleOwnershipOption();

            double getExpOneCar(int unitTypeId);

            double getExpTwoPlusCar(int unitTypeId);

            /*
             * check all the vehicle categories and returns if it includes a motorcycle
             */
            bool isMotorCycle(int vehicleCategoryId);

            void setTaxiAccess();

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
            	ASC_ONECAR = 1, ASC_TWO_PLUS_CAR, B_CHINESE_ONECAR, B_CHINESE_TWO_PLUS_CAR, B_HDB_ONECAR, B_HDB_TWO_PLUS_CAR,
            	B_INC1_ONECAR, B_INC1_TWO_PLUS_CAR, B_INC2_ONECAR, B_INC2_TWO_PLUS_CAR, B_INC3_ONECAR, B_INC3_TWO_PLUS_CAR, B_INC4_ONECAR,
            	B_INC4_TWO_PLUS_CAR, B_INC5_ONECAR, B_INC5_TWO_PLUS_CAR, B_INC6_ONECAR, B_INC6_TWO_PLUS_CAR, B_KIDS_ONECAR, B_KIDS_TWO_PLUS_CAR,
            	B_LOG_HHSIZE_ONECAR, B_LOG_HHSIZE_TWO_PLUS_CAR, B_MC_ONECAR, B_MC_TWO_PLUS_CAR
            };
            enum VehicleOwnershipOption{
            	NO_CAR = 1, ONE_CAR, TWO_PLUS_CAR
            };
            enum TaxiAccessParamId{
            	INTERCEPT = 1, HDB1, AGE5064_1, AGE5064_2, AGE65UP_1, AGE65UP_2, AGE3549_2, AGE1019_2, EMPLOYED_SELF_1, EMPLOYED_SELF_2, INC_LOW, INC_HIGH, RETIRED_1, RETIRED_2, OPERATOR_1,
            	OPERATOR_2, SERVICE_2, PROF_1, LABOR_1, MANAGER_1, INDIAN_TAXI_ACCESS, MALAY_TAXI_ACCESS
            };
            VehicleOwnershipOption vehicleOwnershipOption;
            bool hasTaxiAccess;
            float householdAffordabilityAmount;
            bool initBidderRole;
        };
    }
}
