//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DeveloperAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *       : Gishara Premarathne <gishara@smart.mit.edu>
 *
 * Created on Mar 5, 2014, 6:36 PM
 */
#pragma once
#include "agent/LT_Agent.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/Parcel.hpp"
#include "database/entity/PotentialProject.hpp"
#include "database/entity/Building.hpp"
#include "database/entity/Unit.hpp"
#include "database/entity/Project.hpp"
#include "RealEstateAgent.hpp"

namespace sim_mob {

    namespace long_term {

        class DeveloperModel;
        
        class DeveloperAgent : public LT_Agent {
        public:
            DeveloperAgent(Parcel* parcel, DeveloperModel* model);
            virtual ~DeveloperAgent();
            
            /**
             * Assigns a parcel to be processed by this agent.
             * 
             * @param parcelId parcel to process.
             */
            void assignParcel(BigSerial parcelId);

            /**
             * Tells if the agent is active or not.
             * @return true if the agent is active, false otherwise.
             */
            bool isActive() const {
            	return active;
            }

            /**
             * Disable or enable the agent.
             * @param active
             */
            void setActive(bool active) {
            	this->active = active;
            }
       
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
        public:
            enum UnitSaleStatus {
            	UNIT_NOT_LAUNCHED = 1, UNIT_LAUNCHED_BUT_UNSOLD, UNIT_LAUNCHED_AND_SOLD};
            enum UnitPhysicalStatus {
                UNIT_NOT_READY_FOR_OCCUPANCY = 1, UNIT_READY_FOR_OCCUPANCY_AND_VACANT, UNIT_READY_FOR_OCCUPANCY_AND_OCCUPIED};
            enum BuildingStatus{
            	BUILDING_UNCOMPLETED_WITHOUT_PREREQUISITES = 1, BUILDING_UNCOMPLETED_WITH_PREREQUISITES, BUILDING_NOT_LAUNCHED, BUILDING_LAUNCHED_BUT_UNSOLD, BUILDING_LAUNCHED_AND_SOLD, BUILDING_COMPLETED_WITH_PREREQUISITES, BUILDING_DEMOLISHED
            };
            enum UnitStatus{
            	UNIT_PLANNED, UNIT_UNDER_CONSTRUCTION, UNIT_CONSTRUCTION_COMPLETED, UNIT_DEMOLISHED
            };
            /**
             * Events callbacks.
             */
            virtual void onEvent(event::EventId eventId, event::Context ctxId,event::EventPublisher* sender, const event::EventArgs& args);

            /**
            * Processes the given event.
            * @param eventId
            * @param ctxId
            * @param args
            */
            void processEvent(event::EventId eventId, event::Context ctxId,
                                          const event::EventArgs& args);
        
            /*
             * create new units and buildings, do the initial status encoding of parcel,unit and buildings
             * @param project : most profitable project selected for this parcel
             * @param projectId
             */
            void createUnitsAndBuildings(PotentialProject &project, BigSerial projectId);
            /*
             * create fm_project from potential most profitable project
             * param project : most profitable project selected for this parcel
             */
            void createProject(PotentialProject &project, BigSerial projectId);

            /*
             * update the status and date params of buildings and units
             */
            void processExistingProjects();

            /*
             * set a 20% of new units at each month after 6th month of the simulation
             * to be sent to HM via real estate agent
             */
            void setUnitsForHM(std::vector<Unit*>::iterator &first,std::vector<Unit*>::iterator &last);

            void setUnitsRemain (bool unitRemain);
            std::tm getDate(int day);
            void setRealEstateAgent(RealEstateAgent* realEstAgent);

        private:
            DeveloperModel* model;
            Parcel *parcel;
            IdVector parcelsToProcess;
            bool active;
            std::vector<boost::shared_ptr<Building> > newBuildings;
            //std::vector<boost::shared_ptr<Unit> > newUnits;
            std::vector<Unit*> newUnits;
            boost::shared_ptr<Project> fmProject;
            int monthlyUnitCount;
            bool unitsRemain;
            std::vector<BigSerial> toBeDemolishedBuildingIds;
            RealEstateAgent* realEstateAgent;

        };
    }
}

