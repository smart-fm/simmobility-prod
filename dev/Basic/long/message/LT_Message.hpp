//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LT_Message.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 8, 2013, 11:06 AM
 */
#pragma once
#include "Common.hpp"
#include "database/entity/Bid.hpp"
#include "message/Message.hpp"
#include "message/MessageHandler.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a bid response.
         */
        enum BidResponse {
            ACCEPTED,
            NOT_ACCEPTED,
            BETTER_OFFER,// not accepted because the seller has a better offer
            NOT_AVAILABLE,// means that the seller is not the owner of the unit or that unit is not available.
        };

        /**
         * Bid message data to be exchanged on Bid process communication.
         */
        class BidMessage : public messaging::Message {
        public:
            BidMessage(const Bid& bid);
            BidMessage(const Bid& bid, BidResponse response);
            BidMessage(const BidMessage& orig);
            virtual ~BidMessage();
            BidMessage& operator=(const BidMessage& source);

            /**
             * Gets the response if is available.
             * @return {@link BidResponse} value.
             */
            const BidResponse& getResponse() const;

            /**
             * Gets the bid.
             * @return {@link Bid} instance.
             */
            const Bid& getBid()const;
            
        private:
            Bid bid;
            BidResponse response;
        };
    }
}

