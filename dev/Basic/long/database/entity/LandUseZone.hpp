//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   LandUseZone.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 21, 2014, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class LandUseZone {
        public:
            LandUseZone(BigSerial id = INVALID_ID,
                        BigSerial typeId = INVALID_ID,
                        double gpr = 0);

            virtual ~LandUseZone();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            BigSerial getTypeId() const;
            double getGPR() const;

            /**
             * Operator to print the LandUseZone data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const LandUseZone& data);
        private:
            friend class LandUseZoneDao;
        private:
            BigSerial id;
            BigSerial typeId;
            double gpr;
        };
    }
}
