/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 9, 2013, 4:39 PM
 */
#pragma once
#include "HMLuaModel.hpp"

namespace sim_mob {

    namespace long_term {

        class LuaProxy {
        public:
            static HMLuaModel& getHM_Model();
        };
    }
}

