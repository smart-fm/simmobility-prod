/*
 * DistanceMRT.cpp
 *
 *  Created on: Jun 2, 2015
 *      Author: gishara
 */

#include "DistanceMRT.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

DistanceMRT::DistanceMRT(BigSerial houseHoldId,double distanceMrt): houseHoldId(houseHoldId), distanceMrt(distanceMrt){}

DistanceMRT::~DistanceMRT() {
}

BigSerial DistanceMRT::getHouseholdId() const {
    return houseHoldId;
}

double DistanceMRT::getDistanceMrt() const {
    return distanceMrt;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const DistanceMRT& data) {
            return strm << "{"
                    << "\"houseHoldId\":\"" << data.houseHoldId << "\","
                    << "\"distanceMrt\":\"" << data.distanceMrt << "\","
                    << "}";
        }
    }
}



