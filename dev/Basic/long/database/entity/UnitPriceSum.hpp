/*
 * UnitPriceSum.hpp
 *
 *  Created on: Aug 20, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitPriceSum {
        public:
        	UnitPriceSum( BigSerial fmParcelId = INVALID_ID, double unitPriceSum = 0.0);

            virtual ~UnitPriceSum();

            BigSerial getFmParcelId() const;


            double getUnitPriceSum() const;

            /**
             * Getters and Setters
             */


            /**
             * Operator to print the UnitPrice data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const UnitPriceSum& data);
        private:
            friend class UnitPriceSumDao;
        private:
            BigSerial fmParcelId;
            double unitPriceSum;
        };
    }
}


