/*
 * HedonicCoeffsByUnitType.hpp
 *
 *  Created on: 11 Apr 2017
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HedonicCoeffsByUnitType {
        public:
        	HedonicCoeffsByUnitType(BigSerial unitTypeId = INVALID_ID,double intercept = 0, double logSqrtArea = 0,double freehold = 0, double logsumWeighted = 0,
        				  double pms_1km = 0, double distanceMallKm = 0,double mrt_200m = 0,double mrt_2_400m = 0,double express_200m = 0,double bus2_400m = 0, double busGt400m = 0,  double age = 0,
						  double ageSquared = 0, double misage = 0);

            virtual ~HedonicCoeffsByUnitType();

            /**
             * Getters and Setters
             */
			double getAge() const;
			double getBus2400m() const;
			double getBusGt400m() const;
			double getDistanceMallKm() const;
			double getExpress200m() const;
			double getFreehold() const;
			double getIntercept() const;
			double getAgeSquared() const;
			double getLogSqrtArea() const;
			double getLogsumWeighted() const;
			double getMisage() const;
			double  getMrt200m() const;
			double getMrt2400m() const;
			double getPms1km() const;
			BigSerial getUnitTypeId() const;

			void setAge(double age);
			void setBus2400m(double bus2400m);
			void setBusGt400m(double busGt400m);
			void setDistanceMallKm(double distanceMallKm);
			void setExpress200m(double express200m);
			void setFreehold(double freehold) ;
			void setIntercept(double intercept);
			void setAgeSquared(double ageSquared);
			void setLogSqrtArea(double logSqrtArea);
			void setLogsumWeighted(double logsumWeighted);
			void setMisage(double misage);
			void setMrt200m(double mrt200m);
			void setMrt2400m(double mrt2400m);
			void setPms1km(double pms1km);
			void setUnitTypeId(BigSerial unitTypeId);

        private:

            BigSerial unitTypeId;
            double intercept;
            double logSqrtArea;
            double freehold;
            double logsumWeighted;
            double pms_1km;
            double distanceMallKm;
            double mrt_200m;
            double mrt_2_400m;
            double express_200m;
            double bus2_400m;
            double busGt400m;
            double age;
            double ageSquared;
            double misage;
        };
    }
}
