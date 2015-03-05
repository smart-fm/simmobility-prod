/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *       : Gishara Premarathne <gishara@smart.mit.edu>
 *
 * Created on March 11, 2014, 3:08 PM
 */
#pragma once
#include "boost/unordered_map.hpp"
#include "Model.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/Template.hpp"
#include "database/entity/Parcel.hpp"
#include "database/entity/LandUseZone.hpp"
#include "database/entity/DevelopmentTypeTemplate.hpp"
#include "database/entity/TemplateUnitType.hpp"
#include "database/entity/Project.hpp"
#include "database/entity/ParcelMatch.hpp"
#include "database/entity/SlaParcel.hpp"
#include "database/entity/UnitType.hpp"
#include "database/entity/Building.hpp"
#include "database/entity/TotalBuildingSpace.hpp"
#include "database/entity/ParcelAmenities.hpp"
#include "database/entity/MacroEconomics.hpp"
#include "agent/impl/DeveloperAgent.hpp"
#include "model/HM_Model.hpp"

namespace sim_mob {
    namespace long_term {

        class DeveloperModel : public Model {
        public:

            typedef std::vector<DeveloperAgent*> DeveloperList;
            typedef std::vector<Template*> TemplateList;
            typedef std::vector<Parcel*> ParcelList;
            typedef std::vector<LandUseZone*> LandUseZonesList;
            typedef std::vector<DevelopmentTypeTemplate*> DevelopmentTypeTemplateList;
            typedef std::vector<TemplateUnitType*> TemplateUnitTypeList;
            typedef std::vector<UnitType*> UnitTypeList;
            typedef std::vector<Building*> BuildingList;
            typedef std::vector<TotalBuildingSpace*> BuildingSpaceList;
            typedef std::vector<Project*> ProjectList;
            typedef std::vector<ParcelAmenities*> AmenitiesList;
            typedef std::vector<MacroEconomics*> MacroEconomicsList;
            //maps
            typedef boost::unordered_map<BigSerial,Parcel*> ParcelMap;
            typedef boost::unordered_map<BigSerial,UnitType*> UnitTypeMap;
            typedef boost::unordered_map<BigSerial,TotalBuildingSpace*> TotalBuildingSpaceMap;
            typedef boost::unordered_map<BigSerial,ParcelAmenities*> AmenitiesMap;
            typedef boost::unordered_map<BigSerial,MacroEconomics*> MacroEconomicsMap;

        public:
            DeveloperModel(WorkGroup& workGroup);
            DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel );
            virtual ~DeveloperModel();
            
            /*
             * create developer agents for each parcel in the given ParcelList
             */
            void createDeveloperAgents(ParcelList initParcelList);

            void wakeUpDeveloperAgents(DeveloperList devAgentList);

            /*
             * process the initial parcel list e to create sub parcel lists as follow:
             * 1. parcelsWithProjectsList 2. developmentCandidateParcelList 3. nonEligibleParcelList
             *
             */
            void processParcels();

            /*
             * check whether a project is older than 90 days or more.
             * if so, add it to the developmentCandidateParcelList if the parcel is not in the list yet.
             */
            void processProjects();

            /**
             * Getters 
             */
            unsigned int getTimeInterval() const;
            Parcel* getParcelById(BigSerial id) const;
            SlaParcel* getSlaParcelById(std::string id) const;
            const LandUseZone* getZoneById(BigSerial id) const;
            const DevelopmentTypeTemplateList& getDevelopmentTypeTemplates() const;
            const TemplateUnitTypeList& getTemplateUnitType() const;
            const UnitType* getUnitTypeById(BigSerial id) const;
            const ParcelAmenities* getAmenitiesById(BigSerial fmParcelId) const;
            const MacroEconomics* getMacroEconById(BigSerial id) const;
            float getBuildingSpaceByParcelId(BigSerial id) const;
            ParcelList getDevelopmentCandidateParcels(bool isInitial);

            DeveloperList getDeveloperAgents(bool isInitial);
            /*
             * set the iterators of development candidate parcels, at each time tick (day)
             */
            void setIterators(ParcelList::iterator &first,ParcelList::iterator &last,bool isInitial);

            void setDevAgentListIterator(DeveloperList::iterator &first,DeveloperList::iterator &last,bool isInitial);
            /*
             * set the boolean parameter to indicate whether there are remaining parcels in the pool
             */
            void setIsParcelsRemain(bool parcelStatus);

            bool getIsParcelRemain();

            void setIsDevAgentsRemain(bool parcelStatus);

            /*
             * @param days number of days the simulation runs
             */
            void setDays(int days);

            /*
             * reload the zones from the db if a zoning rule change event occurs
             */
            void reLoadZonesOnRuleChangeEvent();

            //calculate the actual gpr of a given parcel
            //float getActualGpr(Parcel &parcel);

            //get the allowed gpr of zoning.
            float getAllowedGpr(Parcel &parcel);

            /**
            * check whether a given parcel is empty or not
            * @param id of the given parcel
            */
            const bool isEmptyParcel(BigSerial id) const;
            /*
             * increment the id of the last project in db
             * @return next projectId
             */
            BigSerial getProjectIdForDeveloperAgent();
            /*
             * increment the id of the last building in db
             * @return next buildingId.
             */
            BigSerial getBuildingIdForDeveloperAgent();
            /*
             * increment the id of the last building in db
             * @return next unitId.
             */
            boost::recursive_mutex m_guard;
            BigSerial getUnitIdForDeveloperAgent();
            Unit* 	  makeNewUnit(std::vector<PotentialUnit>::iterator unitsItr, std::tm toDate);

            void setUnitId(BigSerial unitId);
            /*
             * @return the loaded building list
             */
            BuildingList getBuildings();
            /*
             * add the building id's of the demolished buildings to preserve and add to new buildings later
             * @param building Id of the demolished building
             */
            void addNewBuildingId(BigSerial buildingId);
            /*
             * set the current tick of the simulation from main
             * @param current tick
             */
            void setCurrentTick(int currTick);
            /*
             * get the current tick of the simulation
             * @return current tick
             */
            int getCurrentTick();

            void addProjects(boost::shared_ptr<Project> project);

            void addBuildings(boost::shared_ptr<Building> building);

        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();

        private:
            DeveloperList developers;
            TemplateList templates;
            ParcelList initParcelList;
            ParcelList parcelsWithProjectsList;
            ParcelList developmentCandidateParcelList;
            ParcelList nonEligibleParcelList;
            ParcelList emptyParcels;
            BuildingList buildings;
            std::vector<boost::shared_ptr<Building> > newBuildings;
            DevelopmentTypeTemplateList developmentTypeTemplates;
            TemplateUnitTypeList templateUnitTypes;
            BuildingSpaceList buildingSpaces;
            ProjectList projects;
            std::vector<boost::shared_ptr<Project> > newProjects;
            ParcelMap parcelsById;
            ParcelMap emptyParcelsById;
            ParcelMap devCandidateParcelsById;
            unsigned int timeInterval;
            std::vector<BigSerial> existingProjectIds;
            std::vector<BigSerial> newBuildingIdList;
            UnitTypeList unitTypes;
            UnitTypeMap unitTypeById;
            TotalBuildingSpaceMap buildingSpacesByParcelId;
            int dailyParcelCount;
            int dailyAgentCount;
            bool isParcelRemain;
            bool isDevAgentsRemain;
            int numSimulationDays;
            BigSerial projectId;
            BigSerial buildingId;
            BigSerial unitId;
            AmenitiesList amenities;
            AmenitiesMap amenitiesById;
            int currentTick;
            MacroEconomicsList macroEconomics;
            MacroEconomicsMap macroEconomicsById;

            std::vector<Unit*> newUnits;
        };
    }
}
