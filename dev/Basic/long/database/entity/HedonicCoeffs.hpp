/*
 * HedonicCoeffs.hpp
 *
 *  Created on: 29 Aug 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HedonicCoeffs {
        public:
        	HedonicCoeffs(BigSerial propertyTypeId = INVALID_ID,double intercept = 0, double logSqrtArea = 0,double freehold = 0, double logsumWeighted = 0,
        				  double pms_1km = 0, double distanceMallKm = 0,double mrt_200m = 0,double mrt_2_400m = 0,double express_200m = 0,double bus2_400m = 0, double busGt400m = 0,  double age = 0,
						  double logAgeSquared = 0, double agem25_50 = 0, double agem50 = 0, double misage = 0, double age_30m = 0);

            virtual ~HedonicCoeffs();

            /**
             * Getters and Setters
             */

            double getAge() const;
            double getAge30m() const;
            double getAgem25_50() const;
            double getAgem50() const;
            double getBus400m() const;
            double getBusGt400m() const ;
            double getDistanceMallKm() const;
            double getExpress200m() const;
            double getFreehold() const;
            BigSerial getPropertyTypeId() const ;
            double getIntercept() const;
            double getLogAgeSquared() const;
            double getLogSqrtArea() const;
            double getLogsumWeighted() const;
            double getMisage() const;
            double getMrt_2_400m() const;
            double getMrt200m() const ;
            double getPms1km() const;

            void setAge30m(double age30m);
            void setAge(double age);
            void setAgem25_50(double agem);
            void setAgem50(double agem50);
            void setBus400m(double bus400m);
            void setBusGt400m(double busGt400m);
            void setDistanceMallKm(double distanceMallKm);
            void setExpress200m(double express200m);
            void setFreehold(double freehold);
            void setPropertyTypeId(BigSerial id);
            void setIntercept(double intercept);
            void setLogAgeSquared(double logAgeSquared);
            void setLogSqrtArea(double logSqrtArea);
            void setLogsumWeighted(double logsumWeighted);
            void setMisage(double misage);
            void setMrt_2_400m(double mrt2400m);
            void setMrt200m(double mrt200m);
            void setPms1km(double pms1km);


        private:

            BigSerial proprtyTypeId;
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
            double logAgeSquared;
            double agem25_50;
            double agem50;
            double misage;
            double age_30m;
        };
    }
}
