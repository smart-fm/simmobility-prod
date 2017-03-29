//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   PostcodeAmenities.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * Author: Gishara Premarathne <gishara@smart.mit.edu>
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
            BigSerial getTazId() const;
            BigSerial getAddressId() const;
            const std::string& getMrtStation() const;
            const std::string& getPostcode() const;

            void setBuildingName(const std::string& buildingName);
            void setBus200m(bool bus200m);
            void setBus400m(bool bus400m);
            void setDistanceToBus(double distanceToBus);
            void setDistanceToCbd(double distanceToCbd);
            void setDistanceToExpress(double distanceToExpress);
            void setDistanceToJob(double distanceToJob);
            void setDistanceToMall(double distanceToMall);
            void setDistanceToMrt(double distanceToMrt);
            void setDistanceToPms30(double distanceToPms30);
            void setExpress200m(bool express200m);
            void setMrt200m(bool mrt200m);
            void setMrt400m(bool mrt400m);
            void setMrtStation(const std::string& mrtStation);
            void setPms1km(bool pms1km);
            void setPostcode(const std::string& postcode);
            void setAddressId(BigSerial addressId);
            void setTazId(BigSerial tazId);

            /**
             * Operator to print the PostcodeAmenities data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const PostcodeAmenities& data);

        private:
            friend class PostcodeAmenitiesDao;
        private:
            BigSerial addressId;
            std::string postcode;
            BigSerial tazId;
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
        };
    }
}
