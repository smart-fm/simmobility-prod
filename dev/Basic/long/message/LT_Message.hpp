/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Message.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 8, 2013, 11:06 AM
 */
#pragma once
#include "Common.h"
#include "model/Bid.hpp"
#include "message/Message.hpp"

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

        /**
         * Bid message data to be exchanged on Bid process communication.
         */
        class BidMessage : public Message {
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
            const BidResponse GetResponse() const;

            /**
             * Gets the bid.
             * @return {@link Bid} instance.
             */
            const Bid& GetBid()const;
        protected:

            virtual int GetId() {
                return 13;
            }
        private:
            Bid bid;
            BidResponse response;

        };
    }
}

