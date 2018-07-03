//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * BuildingAvgAge.hpp
 *
 *  Created on: 23 Feb 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {

        class BuildingAvgAgePerParcel
        {
        public:
        	BuildingAvgAgePerParcel( BigSerial fmParcelId = INVALID_ID, int age = 0);

            virtual ~BuildingAvgAgePerParcel();

            BuildingAvgAgePerParcel( const BuildingAvgAgePerParcel &source);
            BuildingAvgAgePerParcel& operator=(const BuildingAvgAgePerParcel& source);

            /*
             * getters
             */
            BigSerial getFmParcelId() const;
            int getAge() const;

            /*
             * setters
             */
            void setFmParcelId(BigSerial parcelId);
            void setAge(int age);


            /**
             * Operator to print the BuildingAvgAge data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const BuildingAvgAgePerParcel& data);

        private:
            friend class BuildingAvgAgePerParcelDao;

            BigSerial fmParcelId;
            int age;

        };
    }
}
