/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
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

namespace sim_mob {
    namespace long_term {

        class DeveloperModel : public Model {
        public:

            typedef std::vector<Developer*> DeveloperList;
            typedef std::vector<Template*> TemplateList;
            typedef std::vector<Parcel*> ParcelList;
            typedef std::vector<LandUseZone*> LandUseZonesList;
            typedef std::vector<DevelopmentTypeTemplate*> DevelopmentTypeTemplateList;
            typedef std::vector<TemplateUnitType*> TemplateUnitTypeList;
            typedef std::vector<Project*> ProjectList;
            typedef std::vector<ParcelMatch*> ParcelMatchList;
            typedef std::vector<SlaParcel*> SlaParcelList;
            //maps
            typedef boost::unordered_map<BigSerial,Parcel*> ParcelMap;
            typedef boost::unordered_map<BigSerial,LandUseZone*> LandUseZoneMap;
            typedef boost::unordered_map<BigSerial,ParcelMatch*> ParcelMatchMap;
            typedef boost::unordered_map<BigSerial,Project*> ProjectMap;
            typedef boost::unordered_map<BigSerial,SlaParcel*> SlaParcelMap;

        public:
            DeveloperModel(WorkGroup& workGroup);
            DeveloperModel(WorkGroup& workGroup, unsigned int timeIntervalDevModel );
            virtual ~DeveloperModel();
            
            /*
             * create developer agents for each parcel in the given ParcelList
             */
            void createDeveloperAgents(ParcelList initParcelList);

            /*
             * process the initial parcel list e to create sub parcel lists as follow:
             * 1. parcelsWithProjectsList 2. developmentCandidateParcelList 3. nonEligibleParcelList
             *
             */
            void processParcels();

            /**
             * Getters 
             */
            unsigned int getTimeInterval() const;
            Parcel* getParcelById(BigSerial id) const;
            SlaParcel* getSlaParcelById(BigSerial id) const;
            const LandUseZone* getZoneById(BigSerial id) const;
            const DevelopmentTypeTemplateList& getDevelopmentTypeTemplates() const;
            const TemplateUnitTypeList& getTemplateUnitType() const;

            /*
             * returns the sla parcel id of a parcel, given the fm parcel id
             */
            BigSerial getSlaParcelIdByFmParcelId(BigSerial fmParcelId) const;

            /*
             * check whether a given parcel is already associated with a project
             */
            bool isParcelWithExistingProject(const Parcel *parcel) const;

            void setParcelMatchMap(ParcelMatchMap parcelMatchMap);
            ParcelList getDevelopmentCandidateParcels();

            /*
             * set the iterators of development candidate parcels, at each time tick (day)
             */
            void setIterators(ParcelList::iterator &first,ParcelList::iterator &last);

            /*
             * set the boolean parameter to indicate whether there are remaining parcels in the pool
             */
            void setIsParcelsRemain(bool parcelStatus);

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
            ProjectList existingProjects;
            LandUseZonesList zones;
            DevelopmentTypeTemplateList developmentTypeTemplates;
            TemplateUnitTypeList templateUnitTypes;
            ParcelMap parcelsById;
            LandUseZoneMap zonesById;
            unsigned int timeInterval;
            ParcelMatchList parcelMatches;
            ParcelMatchMap parcelMatchesMap;
            ProjectMap existingProjectMap;
            std::vector<BigSerial> existingProjectParcelIds;
            SlaParcelList slaParcels;
            SlaParcelMap slaParcelById;
            int dailyParcelCount;
            bool isParcelRemain;

        };
    }
}
