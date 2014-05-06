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
            //maps
            typedef boost::unordered_map<BigSerial,Parcel*> ParcelMap;
            typedef boost::unordered_map<BigSerial,LandUseZone*> LandUseZoneMap;
        public:
            DeveloperModel(WorkGroup& workGroup);
            DeveloperModel(WorkGroup& workGroup, unsigned int timeInterval );
            virtual ~DeveloperModel();
            
            /**
             * Getters 
             */
            unsigned int getTimeInterval() const;
            const Parcel* getParcelById(BigSerial id) const;
            const LandUseZone* getZoneById(BigSerial id) const;
            const DevelopmentTypeTemplateList& getDevelopmentTypeTemplates() const;
            const TemplateUnitTypeList& getTemplateUnitType() const;
            
        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();
        private:
            DeveloperList developers;
            TemplateList templates;
            ParcelList parcels;
            LandUseZonesList zones;
            DevelopmentTypeTemplateList developmentTypeTemplates;
            TemplateUnitTypeList templateUnitTypes;
            ParcelMap parcelsById;
            LandUseZoneMap zonesById;
            unsigned int timeInterval;
        };
    }
}
