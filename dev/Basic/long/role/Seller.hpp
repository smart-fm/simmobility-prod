/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Seller.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 5:13 PM
 */
#pragma once
#include "role/Role.hpp"
#include "event/LT_EventArgs.h"


namespace sim_mob {
    
    namespace long_term {

        class UnitHolder;
        
        class Seller : public Role {
        public:
            Seller(UnitHolder* parent);
            virtual ~Seller();
            virtual void Update(timeslice currTime);
            virtual void OnBidReceived(EventId id, Context ctx, 
                EventPublisher* sender, const BidEventArgs& args);
            
        private:
            UnitHolder* cParent; // concrete parent
        };
    }
}

