/*
 * SlaParcel.hpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gishara
 */

#pragma once

#include <ctime>
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
    namespace long_term
    {
        class SlaParcel
        {
        public:
        	SlaParcel(std::string slaId = EMPTY_STR,
            		    BigSerial tazId = INVALID_ID,
            		    BigSerial landUseZoneId = INVALID_ID,
            		    double area = 0,
            		    double length = 0,
            		    double minX = 0,
            		    double minY = 0,
            		    double maxX = 0,
            		    double maxY = 0);

            virtual ~SlaParcel();

            /**
             * Getters and Setters
             */
            std::string getSlalId() const;
            BigSerial getTazId() const;
            BigSerial getLandUseZoneId() const;
            double getArea() const;
            double getLength() const;
            double getMinX() const;
            double getMinY() const;
            double getMaxX()const;
            double getMaxY() const;

            /**
             * Operator to print the Parcel Match data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const SlaParcel& data);
        private:
            friend class SlaParcelDao;

        private:
            std::string slaId;
            BigSerial tazId;
            BigSerial landUseZoneId;
            double area;
            double length;
            double minX;
            double minY;
            double maxX;
            double maxY;
         };
    }
}
