/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HM_Model.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 21, 2013, 3:08 PM
 */
#pragma once
#include "Model.hpp"
#include "core/HousingMarket.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Class that contains Housing market model logic.
         */
        class HM_Model : public Model{
        public:
            HM_Model(WorkGroup& workGroup);
            virtual ~HM_Model();
        protected:
            /**
             * Inherited from Model.
             */
            void startImpl();
            void stopImpl();
        
        private:
            // Data
            HousingMarket market;
        };
    }
}

