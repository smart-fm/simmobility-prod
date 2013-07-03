/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   SellerParams.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 3:05 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Class that contains params like weights about unit fields.
         */
        class SellerParams {
        public:
            SellerParams(BigSerial householdId = INVALID_ID,
                    double priceImportance = .0f, double expectedEvents = .0f);
            virtual ~SellerParams();

            /**
             * An operator to allow the unit copy.
             * @param source an Seller to be copied.
             * @return The Seller after modification
             */
            SellerParams& operator=(const SellerParams& source);

            /**
             * @return value with Household identifier.
             */
            BigSerial GetHouseholdId() const;

            /**
             * @return price importance weight.
             */
            double GetPriceImportance() const;

            /**
             * @return expected events weight.
             */
            double GetExpectedEvents() const;

            /**
             * Operator to print the SellerParam data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const SellerParams& data) {
                return strm << "{"
                        << "\"priceImportance\":\"" << data.priceImportance << "\","
                        << "\"expectedEvents\":\"" << data.expectedEvents << "\""
                        << "}";
            }
        private:
            friend class SellerParamsDao;

        private:
            BigSerial householdId;
            double priceImportance;
            double expectedEvents;
        };
    }
}
