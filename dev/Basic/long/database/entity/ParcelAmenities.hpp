/*
 * ParcelAmenities.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: gishara
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * ParcelAmenities plain object.
         */
        class ParcelAmenities {
        public:
            ParcelAmenities();
            virtual ~ParcelAmenities();

            /**
             * Getters
             */
            int hasPms_1km() const;
            int hasBus_400m() const;
            int hasBus_200m() const;
            int hasExpress_200m() const;
            int hasMRT_400m() const;
            int hasMRT_200m() const;
            double getDistanceToJob() const;
            double getDistanceToMall() const;
            double getDistanceToCBD() const;
            double getDistanceToPMS30() const;
            double getDistanceToExpress() const;
            double getDistanceToBus() const;
            double getDistanceToMRT() const;
            const std::string& getNearestMRT() const;
            BigSerial getFmParcelId() const;

            /**
             * Operator to print the ParcelAmenities data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const ParcelAmenities& data);

        private:
            friend class ParcelAmenitiesDao;
        private:
            BigSerial fmParcelId;
            std::string nearestMRT;
            double distanceToMRT;
            double distanceToBus;
            double distanceToExpress;
            double distanceToPMS30;
            double distanceToCBD;
            double distanceToMall;
            double distanceToJob;
            int mrt_200m;
            int mrt_400m;
            int express_200m;
            int bus_200m;
            int bus_400m;
            int pms_1km;
        };
    }
}
