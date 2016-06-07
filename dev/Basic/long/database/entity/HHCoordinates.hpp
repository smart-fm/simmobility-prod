/*
 * HHCoordinates.hpp
 *
 *  Created on: 11 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HHCoordinates {
        public:
        	HHCoordinates(BigSerial houseHoldId = INVALID_ID, double centroidX = 0, double centroidY = 0);

            virtual ~HHCoordinates();
            /**
             * Getters and Setters
             */
            double getCentroidX() const;
            double getCentroidY() const;
            BigSerial getHouseHoldId() const;

            void setCentroidX(double centroidX);
            void setCentroidY(double centroidY);
            void setHouseHoldId(BigSerial houseHoldId);

        private:
            friend class HHCoordinatesDao;
        private:
            BigSerial houseHoldId;
            double centroidX;
            double centroidY;
        };
    }
}
