/*
 * TravelTime.hpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class TravelTime {
        public:
        	TravelTime(BigSerial origin = INVALID_ID, BigSerial destination = INVALID_ID, double carTravelTime = 0, double publicTravelTime = 0);

            virtual ~TravelTime();


            /**
             * Getters and Setters
             */
            double getCarTravelTime() const;
            BigSerial getDestination() const;
            BigSerial getOrigin() const;
            double getPublicTravelTime() const;

            void setCarTravelTime(double carTravelTime);
            void setDestination(BigSerial destination);
            void setOrigin(BigSerial origin);
            void setPublicTravelTime(double publicTravelTime);

        private:
            friend class TravelTimeDao;
        private:
            BigSerial origin;
            BigSerial destination;
            double carTravelTime;
            double publicTravelTime;
        };
    }
}
