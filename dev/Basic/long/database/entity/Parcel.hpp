//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Parcel.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 10, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class Parcel {
        public:
            Parcel(BigSerial id = INVALID_ID,
                   BigSerial tazId = INVALID_ID,
                   BigSerial landUseZoneId = INVALID_ID,
                   double area   = .0f,
                   double length = .0f,
                   double minX   = .0f,
                   double minY   = .0f,
                   double maxX   = .0f,
                   double maxY   = .0f);

            virtual ~Parcel();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            BigSerial getTazId() const;
            BigSerial getLandUseZoneId() const;
            double getArea() const;
            double getLength() const;
            double getMinX() const;
            double getMinY() const;
            double getMaxX() const;
            double getMaxY() const;

            /**
             * Operator to print the Parcel data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const Parcel& data);
        private:
            friend class ParcelDao;
        private:
            BigSerial id;
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
