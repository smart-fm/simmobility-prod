//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeAmenities.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 11, 2014, 3:05 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * PostcodeAmenities plain object.
         */
        class PostcodeAmenities {
        public:
            PostcodeAmenities();
            virtual ~PostcodeAmenities();
            
            /**
             * Getters 
             */
            bool isHdb() const;
            bool isPrivate() const;
            bool isEc() const;
            bool isDetached() const;
            bool isSemi() const;
            bool isTerrace() const;
            bool isCondo() const;
            bool isApartment() const;
            bool hasPms_1km() const;
            bool hasBus_400m() const;
            bool hasBus_200m() const;
            bool hasExpress_200m() const;
            bool hasMRT_400m() const;
            bool hasMRT_200m() const;
            double getDistanceToJob() const;
            double getDistanceToMall() const;
            double getDistanceToCBD() const;
            double getDistanceToPMS30() const;
            double getDistanceToExpress() const;
            double getDistanceToBus() const;
            double getDistanceToMRT() const;
            const std::string& getMrtStation() const;
            const std::string& getMtzNumber() const;
            const std::string& getRoadName() const;
            const std::string& getUnitBlock() const;
            const std::string& getBuildingName() const;
            const std::string& getPostcode() const;

            /**
             * Operator to print the PostcodeAmenities data.  
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const PostcodeAmenities& data);

        private:
            friend class PostcodeAmenitiesDao;
        private:
            std::string postcode;
            std::string buildingName;
            std::string unitBlock;
            std::string roadName;
            std::string mtzNumber;
            std::string mrtStation;
            double distanceToMRT;
            double distanceToBus;
            double distanceToExpress;
            double distanceToPMS30;
            double distanceToCBD;
            double distanceToMall;
            double distanceToJob;
            bool mrt_200m;
            bool mrt_400m;
            bool express_200m;
            bool bus_200m;
            bool bus_400m;
            bool pms_1km;
            bool apartment;
            bool condo;
            bool terrace;
            bool semi;
            bool detached;
            bool ec;
            bool _private;
            bool hdb;
        };
    }
}
