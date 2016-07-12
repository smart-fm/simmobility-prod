//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on May 16, 2013, 6:36 PM
 */
#pragma once
#include "core/HousingMarket.hpp"
#include "entities/Agent_LT.hpp"
#include "database/entity/Household.hpp"
#include "event/LT_EventArgs.hpp"
#include "model/HM_Model.hpp"
#include "role/impl/HouseholdBidderRole.hpp"
#include "role/impl/HouseholdSellerRole.hpp"

namespace sim_mob
{
    namespace long_term
    {
        //class HM_Model;
        //class HouseholdBidderRole;
        //class HouseholdSellerRole;
        /**
         * Represents an Long-Term household agent.
         * An household agent has the following capabilities:
         * - Sell units.
         * - Bid units. 
         */
        class HouseholdAgent : public Agent_LT
        {
        public:
            HouseholdAgent(BigSerial id, HM_Model* model, Household* hh, HousingMarket* market, bool marketSeller = false, int day = 0, int householdBiddingWindow = 0, int awakeningDay = 0);
            virtual ~HouseholdAgent();
            
            enum VehicleOwnershipOption
            {
            	NO_VEHICLE = 1, PLUS1_MOTOR_ONLY,OFF_PEAK_CAR_W_WO_MOTOR,NORMAL_CAR_ONLY,NORMAL_CAR_1PLUS_MOTOR,NORMAL_CAR_W_WO_MOTOR
            };
            VehicleOwnershipOption vehicleOwnershipOption;

            //not-thread safe
            void addUnitId (const BigSerial& unitId);
            void removeUnitId (const BigSerial& unitId);
            const IdVector& getUnitIds() const;
            HM_Model* getModel() const;
            HousingMarket* getMarket() const;
            Household* getHousehold() const;

            void setBuySellInterval( int value );
            int getBuySellInterval( ) const;

            void setHouseholdBiddingWindow(int value);
            int getAwakeningDay() const;

            void setAwakeningDay(int _day);

            HouseholdBidderRole* getBidder();
            HouseholdSellerRole* getSeller();

            bool getFutureTransitionOwn();
        
        protected:
            /**
             * Inherited from LT_Agent.
             */
            bool onFrameInit(timeslice now);
            sim_mob::Entity::UpdateStatus onFrameTick(timeslice now);
            void onFrameOutput(timeslice now);
            
            /**
             * Inherited from Entity. 
             */
            void onWorkerEnter();
            void onWorkerExit();
            virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);
        private:
            /**
             * Method called to find some unit and bid it.
             * @param now current time.
             * @return true if some bid was sent, false otherwise.
             */
            bool bidUnit(timeslice now);
            
            /**
             * Events callbacks.
             */
            virtual void onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
            
            /**
             * Processes the given event.
             * @param eventId
             * @param ctxId
             * @param args
             */
            void processEvent(event::EventId eventId, event::Context ctxId, const event::EventArgs& args);
            
            /**
             * Processes external event.
             * @param args
             */
            void processExternalEvent(const ExternalEventArgs& args);
            

        private:
            HM_Model* model;
            HousingMarket* market;
            Household* household;

            IdVector unitIds;

            HouseholdBidderRole* bidder;
            HouseholdSellerRole* seller;

            int buySellInterval;

            int householdBiddingWindow;

            bool marketSeller; //tells if the agent is only a fake market seller
            int day;

            bool futureTransitionOwn; //If awakened, will the household choose to rent or own a unit? If true, this household will choose to own.

            int awakeningDay;

        };
    }
}

