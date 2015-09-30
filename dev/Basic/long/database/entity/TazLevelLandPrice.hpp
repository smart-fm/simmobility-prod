//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * TazLevelLandPrice.hpp
 *
 *  Created on: Sep 3, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class TazLevelLandPrice
        {
        public:
        	TazLevelLandPrice(BigSerial tazId = INVALID_ID, float landValue = 0.0);

            virtual ~TazLevelLandPrice();

            /**
             * Getters and Setters
             */
            float getLandValue() const;
            void setLandValue(float landVal);
            BigSerial getTazId() const;
            void setTazId(BigSerial tazId);

            /**
             * Operator to print the Template data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const TazLevelLandPrice& data);

        private:
            friend class TazLevelLandPriceDao;

            BigSerial tazId;
            float landValue;
        };
    }
}
