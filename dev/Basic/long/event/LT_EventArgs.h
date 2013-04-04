/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_EventArgs.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 4, 2013, 5:42 PM
 */
#pragma once
#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a bid response.
         */
        enum BidResponse {
            NOT_AVAILABLE,
            ACCEPTED,
            NOT_ACCEPTED
        };

        DECLARE_CUSTOM_CALLBACK_TYPE(BidEventArgs)
        class BidEventArgs : public EventArgs {
        public:
            BidEventArgs(int bidderId, double bid);
            BidEventArgs(int bidderId, BidResponse response);
            BidEventArgs(const BidEventArgs& orig);
            virtual ~BidEventArgs();
            const BidResponse GetResponse() const;
            const double GetBid()const;
            const int GetBidderId()const;
        private:
            BidResponse response;
            int bidderId;
            double bidValue;
        };
    }
}

