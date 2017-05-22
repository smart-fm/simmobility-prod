/*
 * LagPrivate_TByUnitType.hpp
 *
 *  Created on: 17 May 2017
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class LagPrivate_TByUnitType {
        public:
        	LagPrivate_TByUnitType(BigSerial unitTypeId = INVALID_ID, double intercept = 0, double t4 = 0, double t5 = 0, double t6 = 0, double t7 = 0, double gdpRate = 0);

            virtual ~LagPrivate_TByUnitType();

            /**
             * Getters and Setters
             */
			double getGdpRate() const;
			double getIntercept() const;
			double getT4() const;
			double getT5() const;
			double getT6() const;
			double getT7() const;
			BigSerial getUnitTypeId() const;

			void setGdpRate(double gdpRate);
			void setIntercept(double intercept);
			void setT4(double t4);
			void setT5(double t5);
			void setT6(double t6);
			void setT7(double t7);
			void setUnitTypeId(BigSerial unitTypeId);


        private:

            BigSerial unitTypeId;
            double intercept;
            double t4;
            double t5;
            double t6;
            double t7;
            double gdpRate;

        };
    }
}
