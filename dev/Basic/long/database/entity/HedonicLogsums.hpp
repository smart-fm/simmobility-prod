/*
 * HedonicLogsums.hpp
 *
 *  Created on: 27 Sep 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class HedonicLogsums {
        public:
        	HedonicLogsums(BigSerial tazId = INVALID_ID,double logsumWeighted = 0);

            virtual ~HedonicLogsums();

            /**
             * Getters and Setters
             */

            double getLogsumWeighted() const;
            BigSerial getTazId() const;

			void setLogsumWeighted(double logsumWeighted);
			void setTazId(BigSerial tazId);

        private:

            BigSerial tazId;
            double logsumWeighted;

        };
    }
}

