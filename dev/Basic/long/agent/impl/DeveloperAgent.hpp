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
        private:
            enum UnitSaleStatus {
            	NOT_LAUNCHED, LAUNCHED_BUT_UNSOLD, LAUNCHED_AND_SOLD};
            enum UnitPhysicalStatus {
                        	NOT_READY_FOR_OCCUPANCY, READY_FOR_OCCUPANCY_AND_VACANT, READY_FOR_OCCUPANCY_AND_OCCUPIED};
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

        private:
            DeveloperModel* model;
            Parcel *parcel;
            IdVector parcelsToProcess;
            bool active;
            std::vector<Building> newBuildings;
            std::vector<Unit> newUnits;
            Project *fmProject;

        };
    }
}

