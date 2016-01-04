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
#include "entities/Agent_LT.hpp"

namespace sim_mob {

    namespace long_term {

        class LT_Agent : public sim_mob::Agent_LT
		{
        public:
            LT_Agent(int id);
            virtual ~LT_Agent();

            /**
             * Inherited from Agent.
             */
            //virtual void load(const std::map<std::string, std::string>& configProps);

        protected:


            //sim_mob::Entity::UpdateStatus update(timeslice now);


        };
    }
}

