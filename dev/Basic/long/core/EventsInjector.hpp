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

namespace sim_mob {

    namespace long_term {

        class EventsInjector : public Entity {
        public:
            EventsInjector();
            virtual ~EventsInjector();
            
            /**
             * Inherited from Entity
             */
            virtual UpdateStatus update(timeslice now);
            
        protected:
            /**
             * Inherited from Entity
             */
            virtual bool isNonspatial();
            virtual void buildSubscriptionList(
                std::vector<BufferedBase*>& subsList);

        private:
            /**
             * Inherited from Entity
             */
            void onWorkerEnter();
            void onWorkerExit();
        };
    }
}