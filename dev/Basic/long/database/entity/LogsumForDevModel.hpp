/*
 * LogsumForDevModel.hpp
 *
 *  Created on: May 13, 2015
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class LogsumForDevModel {
        public:
        	LogsumForDevModel( BigSerial fmParcelId = INVALID_ID, BigSerial tazId = INVALID_ID, double accessibility = 0);

            virtual ~LogsumForDevModel();

            /**
             * Getters and Setters
             */
            BigSerial getFmParcelId() const;
            BigSerial getTazId() const;
            double getAccessibility() const;

            /**
             * Operator to print the LandUseZone data.
             */
            //friend std::ostream& operator<<(std::ostream& strm, const LogsumForDevModel& data);
        private:
            friend class LogsumForDevModelDao;
        private:
            BigSerial fmParcelId;
            BigSerial tazId;
            double accessibility;
        };
    }
}
