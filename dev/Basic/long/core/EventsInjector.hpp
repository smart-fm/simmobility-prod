/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventsInjector.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 8, 2013, 1:32 PM
 */
#pragma once

#include "entities/Entity.hpp"
#include "model/HM_Model.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Entity responsible to inject some external events
         * to affect the models simulation.
         * 
         * For now, this implementation calls a bunch of lua functions 
         * that will provide the events for the given day.
         * 
         */
        class EventsInjector : public Entity {
        public:
            EventsInjector();
            virtual ~EventsInjector();
            
            /**
             * Inherited from Entity
             */
            virtual UpdateStatus update(timeslice now);
            
            void setModel(HM_Model *value);
            HM_Model* getModel();

        protected:
            /**
             * Inherited from Entity
             */
            virtual bool isNonspatial();
            virtual std::vector<sim_mob::BufferedBase*> buildSubscriptionList();

        private:
            /**
             * Inherited from Entity
             */
            void onWorkerEnter();
            void onWorkerExit();

            HM_Model *model;
        };
    }
}