/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HMLuaModel.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 10, 2013, 2:39 PM
 */
#pragma once
#include <vector>
#include "lua/LuaModel.hpp"
#include "database/entity/Unit.hpp"
#include "Types.hpp"

namespace sim_mob {
    namespace long_term {

        class HMLuaModel : public lua::LuaModel {
        public:
            HMLuaModel();
            HMLuaModel(const HMLuaModel& orig);
            virtual ~HMLuaModel();
            
            void calulateSellerUnitExpectations(const Unit& unit, std::vector<ExpectationEntry>& outValues);
        private:
            void mapClasses();

        };
    }
}

