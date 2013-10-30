//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include "Common.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents an Long-Term agent.
         * These agents can have different behaviors:
         * - Household
         * - Firm
         * - Developer
         * - etc...
         * It will depend of the context.
         */
        class LT_Agent : public sim_mob::Agent{
        public:
            LT_Agent(int id);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const std::map<std::string, std::string>& configProps);

        protected:
            
            /**
             * Handler for frame_init method from agent.
             * @param now time.
             * @return true if the init ran well or false otherwise.
             */
            virtual bool onFrameInit(timeslice now) = 0;

            /**
             * Handler for frame_tick method from agent.
             * 
             * @param now time.
             * @return update status.
             */
            virtual sim_mob::Entity::UpdateStatus onFrameTick(timeslice now) = 0;

            /**
             * Handler for frame_output method from agent.
             * @param now time.
             */
            virtual void onFrameOutput(timeslice now) = 0;

            /**
             * Inherited from Agent.
             */
            bool frame_init(timeslice now);
            sim_mob::Entity::UpdateStatus frame_tick(timeslice now);
            void frame_output(timeslice now);
            bool isNonspatial();    
        };
    }
}

