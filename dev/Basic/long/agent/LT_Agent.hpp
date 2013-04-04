/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include <queue>
#include <string>
#include "entities/UpdateParams.hpp"
#include "conf/simpleconf.hpp"
#include "entities/Agent.hpp"
#include "event/EventManager.hpp"
#include "Common.h"

using namespace sim_mob;
using std::queue;
using std::vector;
using std::string;

namespace sim_mob {
    
    namespace long_term {

        class LT_Agent : public Agent {
        public:
            LT_Agent(int id = -1);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const std::map<std::string, std::string>& configProps);
            
            /**
             * Gets the EventManager reference from worker parent.
             * @return EventManager reference. 
             */
            EventManager& GetEventManager();
        
        protected:
            /**
             * Inherited from Agent.
             */
            virtual bool frame_init(timeslice now);
            virtual Entity::UpdateStatus frame_tick(timeslice now);
            virtual void frame_output(timeslice now);
            virtual bool isNonspatial();
            virtual bool Update(timeslice now) = 0;
            
            
        private:
            queue<int> messages;
        };
    }
}

