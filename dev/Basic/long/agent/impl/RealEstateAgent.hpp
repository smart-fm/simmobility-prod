//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdAgent.hpp
 * Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 *
 * Created on May 16, 2013, 6:36 PM
 */
#pragma once
#include "core/HousingMarket.hpp"
#include "agent/LT_Agent.hpp"
#include "database/entity/Household.hpp"
#include "event/LT_EventArgs.hpp"


namespace sim_mob
{
    namespace long_term
    {
        class HM_Model;
        class RealEstateSellerRole;

        /**
         * Represents an Long-Term realestate agent.
         * An realestate agent has the following capabilities:
         *    - sell the units built by the developer model
         */

        class RealEstateAgent : public LT_Agent
        {
        private:
        	enum BuildingStatus{
        		BUILDING_UNCOMPLETED_WITHOUT_PREREQUISITES = 1, BUILDING_UNCOMPLETED_WITH_PREREQUISITES, BUILDING_NOT_LAUNCHED, BUILDING_LAUNCHED_BUT_UNSOLD, BUILDING_LAUNCHED_AND_SOLD, BUILDING_COMPLETED_WITH_PREREQUISITES, BUILDING_DEMOLISHED
        	};
        	enum UnitStatus{
        	    UNIT_PLANNED = 1, UNIT_UNDER_CONSTRUCTION, UNIT_CONSTRUCTION_COMPLETED, UNIT_DEMOLISHED
        	};
        	enum UnitSaleStatus{
        		UNIT_NOT_LAUNCHED = 1, UNIT_LAUNCHED_BUT_UNSOLD, UNIT_LAUNCHED_AND_SOLD
        	};
        	enum UnitPhysicalStatus{
        		UNIT_NOT_READY_FOR_OCCUPANCY = 1, UNIT_READY_FOR_OCCUPANCY_AND_VACANT, UNIT_READY_FOR_OCCUPANCY_AND_OCCUPIED
        	};
        public:
            RealEstateAgent(BigSerial id, HM_Model* model, const Household* hh, HousingMarket* market, bool marketSeller = false, int day = 0);
            virtual ~RealEstateAgent();
            
            //not-thread safe
            void addUnitId (const BigSerial& unitId);
            void removeUnitId (const BigSerial& unitId);
            const IdVector& getUnitIds() const;
            const IdVector& getPreferableZones() const;
            HM_Model* getModel() const;
            HousingMarket* getMarket() const;
            const Household* getHousehold() const;
            void addNewBuildings(Building *building);
            void addNewUnits(Unit *unit);
            void changeToDateInToBeDemolishedBuildings(BigSerial buildingId,std::tm toDate);
            void changeBuildingStatus(BigSerial buildingId,BuildingStatus buildingStatus);
            void changeUnitStatus(BigSerial unitId,UnitStatus unitStatus);
            void changeUnitSaleStatus(BigSerial unitId,UnitSaleStatus unitSaleStatus);
            void changeUnitPhysicalStatus(BigSerial unitId,UnitPhysicalStatus unitPhysicalStatus);

        
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
            const Household* household;
            IdVector unitIds;
            IdVector preferableZones;
            RealEstateSellerRole* seller;
            bool marketSeller; //tells if the agent is only a fake market seller
            int day;
            std::vector<Building*> buildings;
            std::vector<Unit*> units;
            boost::unordered_map<BigSerial,Building*> buildingsById;
            boost::unordered_map<BigSerial,Unit*> unitsById;

        };
    }
}

