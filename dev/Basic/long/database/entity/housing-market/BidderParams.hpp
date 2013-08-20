/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   BidderParams.hpp
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
        class BidderParams {
        public:
            BidderParams(BigSerial householdId = INVALID_ID,
                    double hhIncomeWeight = .0f, double unitTypeWeight = .0f,
                    double unitAreaWeight = .0f, double unitStoreyWeight = .0f,
                    double unitRentWeight = .0f, double urgencyToBuy = .0f,
                    double priceQuality = .0f);
            virtual ~BidderParams();

            /**
             * An operator to allow the unit copy.
             * @param source an Bidder to be copied.
             * @return The Bidder after modification
             */
            BidderParams& operator=(const BidderParams& source);

            /**
             * @return value with Household identifier.
             */
            BigSerial GetHouseholdId() const;

            /**
             * @return household income weight.
             */
            double GetHH_IncomeWeight() const;

            /**
             * @return type weight.
             */
            double GetUnitTypeWeight() const;

            /**
             * @return unit storey weight.
             */
            double GetUnitStoreyWeight() const;

            /**
             * @return unit area weight value.
             */
            double GetUnitAreaWeight() const;

            /**
             * @return rent value weight.
             */
            double GetUnitRentWeight() const;

            /**
             * @return urgency to buy.
             */
            double GetUrgencyToBuy() const;

            /**
             * @return price quality.
             */
            double GetPriceQuality() const;

            /**
             * Operator to print the BidderParam data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const BidderParams& data) {
                return strm << "{"
                        << "\"householdId\":\"" << data.householdId << "\","
                        << "\"hhIncomeWeight\":\"" << data.hhIncomeWeight << "\","
                        << "\"unitTypeWeight\":\"" << data.unitTypeWeight << "\","
                        << "\"unitAreaWeight\":\"" << data.unitAreaWeight << "\","
                        << "\"unitStoreyWeight\":\"" << data.unitStoreyWeight << "\","
                        << "\"unitRentWeight\":\"" << data.unitRentWeight << "\","
                        << "\"urgencyToBuy\":\"" << data.urgencyToBuy << "\","
                        << "\"priceQuality\":\"" << data.priceQuality << "\""
                        << "}";
            }
        private:
            friend class BidderParamsDao;

        private:
            BigSerial householdId;
            double hhIncomeWeight;
            double unitTypeWeight;
            double unitAreaWeight;
            double unitStoreyWeight;
            double unitRentWeight;
            double urgencyToBuy;
            double priceQuality;
        };
    }
}
