//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Postcode.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 11, 2014, 3:05 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "geospatial/coord/CoordinateTransform.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Postcode plain object.
         */
        class Postcode {
        public:
            Postcode(BigSerial id = INVALID_ID, BigSerial tazId = INVALID_ID,
                    const std::string& code = EMPTY_STR, 
                    double latitude = 0, double longitude = 0);
            Postcode(const Postcode& source);
            virtual ~Postcode();
            Postcode& operator=(const Postcode& source);

            /**
             * Getters 
             */
            BigSerial getId() const;
            BigSerial getTazId() const;
            const std::string& getCode() const;
            const LatLngLocation& getLocation() const;

            /**
             * Operator to print the Unit data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const Postcode& data);

        private:
            friend class PostcodeDao;
        private:
            BigSerial id;
            BigSerial tazId;
            std::string code;
            LatLngLocation location;
        };
    }
}
