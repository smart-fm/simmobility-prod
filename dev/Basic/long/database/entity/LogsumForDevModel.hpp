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
        	LogsumForDevModel( BigSerial taz2012Id = INVALID_ID, BigSerial taz2008Id = INVALID_ID, double accessibility = 0);

            virtual ~LogsumForDevModel();

            /**
             * Getters and Setters
             */
            BigSerial gettAZ2012Id() const;
            BigSerial getTaz2008Id() const;
            double getAccessibility() const;

            /**
             * Operator to print the LandUseZone data.
             */
            //friend std::ostream& operator<<(std::ostream& strm, const LogsumForDevModel& data);
        private:
            friend class LogsumForDevModelDao;
        private:
            BigSerial taz2012Id;
            BigSerial taz2008Id;
            double accessibility;
        };
    }
}
