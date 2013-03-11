/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on March 11, 2013, 3:05 PM
 */

#include "Unit.hpp"

namespace sim_mob {
    
    namespace long_term {

        Unit::Unit(unsigned int id) : Entity (id) {
        }

        Unit::~Unit() {
        }

        Entity::UpdateStatus Unit::update(timeslice now) {
        }

        void Unit::buildSubscriptionList(vector<BufferedBase*>& subsList) {
        }
    }
}
