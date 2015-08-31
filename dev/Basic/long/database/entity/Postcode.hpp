//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

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

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Postcode plain object.
         */
        class Postcode
        {
        public:
            Postcode( BigSerial address_id = INVALID_ID, std::string sla_postcode = EMPTY_STR, BigSerial taz_id = INVALID_ID, float longitude = .0,
            		  float latitude = .0, bool primary_postcode = false );

            Postcode(const Postcode& source);

            virtual ~Postcode();
            Postcode& operator=(const Postcode& source);

            /**
             * Getters 
             */
            BigSerial getAddressId() const;
            std::string getSlaPostcode() const;
            BigSerial getTazId() const;
            float getLongitude() const;
            float getLatitude() const;
            bool getPrimaryPostcode() const;

            /**
             * Operator to print the Unit data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Postcode& data);

        private:
            friend class PostcodeDao;

            BigSerial address_id;
            std::string sla_postcode;
            BigSerial taz_id;
            float longitude;
            float latitude;
            bool primary_postcode;

        };
    }
}
