/*
 * TotalBuildingSpace.hpp
 *
 *  Created on: Dec 5, 2014
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{

    namespace long_term
    {

        class TotalBuildingSpace
        {
        public:
        	TotalBuildingSpace( BigSerial fmParcelId = INVALID_ID, double space = 0);

            virtual ~TotalBuildingSpace();

            /**
             * Gets unique identifier of the fm_parcel.
             * @return fm_parcel_id.
             */
            BigSerial getFmParcelId() const;

            /**
             * Gets the total building space in a parcel.
             * @return space.
             */
            float getTotalBuildingSpace() const;


            /**
             * Assign operator.
             * @param source to assign.
             * @return GPR instance reference.
             */
            TotalBuildingSpace& operator=(const TotalBuildingSpace& source);

            /**
             * Operator to print the GPR data.
             */
            friend std::ostream& operator<<(std::ostream& strm, const TotalBuildingSpace& data);

        private:
            friend class TotalBuildingSpaceDao;

            BigSerial fmParcelId;
            double totalBuildingSpace;
        };
    }
}
