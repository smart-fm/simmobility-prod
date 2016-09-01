/*
 * LagPrivateT.hpp
 *
 *  Created on: 1 Sep 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class LagPrivateT {
        public:
        	LagPrivateT(BigSerial propertyTypeId = INVALID_ID, double intercept = 0, double T4 = 0);

            virtual ~LagPrivateT();

            /**
             * Getters and Setters
             */
			BigSerial getPropertyTypeId() const ;
			double getIntercept() const;
			double getT4() const;

			void setPropertyTypeId(BigSerial id);
			void setIntercept(double intercept);
			void setT4(double t4);


        private:

            BigSerial propertyTypeId;
            double intercept;
            double T4;
        };
    }
}
