//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Bid.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 5, 2013, 5:03 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "metrics/Frame.hpp"
#include "agent/LT_Agent.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a bid to a unit.
         */
        class Bid {
        public:
            Bid(BigSerial id, BigSerial bidderId, LT_Agent* bidder, double value, 
                    timeslice& time, double willingnessToPay = .0f, 
                    double surplus = .0f);
            Bid(const Bid& source);
            virtual ~Bid();

            /**
             * An operator to allow the bid copy.
             * @param source an Bid to be copied.
             * @return The Bid after modification
             */
            Bid& operator=(const Bid& source);

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            BigSerial getUnitId() const;

            /**
             * Gets the Bidder unique identifier.
             * @return value with Bidder identifier.
             */
            BigSerial getBidderId() const;
            
            /**
             * Gets the Bidder pointer.
             * @return bidder pointer.
             */
            LT_Agent* getBidder() const;

            /**
             * Gets the value of the bid.
             * @return the value of the bid.
             */
            double getValue() const;
            
            /**
             * Gets the value of the surplus for the current unit.
             * @return the value of the surplus.
             */
            double getSurplus() const;
            
            /**
             * Gets the value of the willingness to pay of the bidder to this unit.
             * @return the value of the willingness to pay.
             */
            double getWillingnessToPay() const;
            
            /**
             * Gets the time of the bid.
             * @return the value of the bid.
             */
            const timeslice& getTime() const;
            
            /**
             * Operator to print the Bid data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, 
                const Bid& data);
            
        private:
            //TODO: FUTURE friend class BidDao;
            /**
             * Private constructor for DAO use. 
             */
            Bid();
        private:
            timeslice time;
            BigSerial unitId;
            BigSerial bidderId;
            double value;
            LT_Agent* bidder;
            
            /**
             * Bidder information. 
             */ 
            double surplus;
            double willingnessToPay;
        };
    }
}

