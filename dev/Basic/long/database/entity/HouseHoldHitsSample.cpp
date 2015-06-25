/*
 * HouseHoldHitsSample.cpp
 *
 *  Created on: Jun 24, 2015
 *      Author: gishara
 */

#include "HouseHoldHitsSample.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

HouseHoldHitsSample::HouseHoldHitsSample(std::string houseHoldHitsId,BigSerial houseHoldId,BigSerial groupId): houseHoldHitsId(houseHoldHitsId),houseHoldId(houseHoldId), groupId(groupId){}

HouseHoldHitsSample::~HouseHoldHitsSample() {
}

std::string HouseHoldHitsSample::getHouseholdHitsId() const {
    return houseHoldHitsId;
}

BigSerial HouseHoldHitsSample::getHouseholdId() const {
    return houseHoldId;
}

BigSerial HouseHoldHitsSample::getGroupId() const {
    return groupId;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const HouseHoldHitsSample& data) {
            return strm << "{"
            		<< "\"houseHoldHitsId\":\"" << data.houseHoldHitsId << "\","
                    << "\"houseHoldId\":\"" << data.houseHoldId << "\","
                    << "\"groupId\":\"" << data.groupId << "\","
                    << "}";
        }
    }
}


