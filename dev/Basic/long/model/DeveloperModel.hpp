/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2014, 3:08 PM
 */
#pragma once
#include "Model.hpp"
#include "database/entity/Developer.hpp"
#include "database/entity/Template.hpp"
#include "database/entity/Parcel.hpp"

namespace sim_mob {
    namespace long_term {

        class DeveloperModel : public Model {
        public:
            typedef std::vector<Developer*> DeveloperList;
            typedef std::vector<Template*> TemplateList;
            typedef std::vector<Parcel*> ParcelList;
        public:
            DeveloperModel(WorkGroup& workGroup);
            virtual ~DeveloperModel();
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
        };
    }
}