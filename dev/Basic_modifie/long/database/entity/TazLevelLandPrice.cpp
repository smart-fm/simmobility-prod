/*
 * TazLevelLandPrice.cpp
 *
 *  Created on: Sep 3, 2015
 *      Author: gishara
 */
#include <database/entity/TazLevelLandPrice.hpp>
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

TazLevelLandPrice::TazLevelLandPrice(BigSerial tazId, float landValue): tazId(tazId), landValue(landValue) {}

TazLevelLandPrice::~TazLevelLandPrice() {}

float TazLevelLandPrice::getLandValue() const
{
		return landValue;
}

void TazLevelLandPrice::setLandValue(float landVal)
{
		this->landValue = landVal;
}

BigSerial TazLevelLandPrice::getTazId() const
{
		return tazId;
}

void TazLevelLandPrice::setTazId(BigSerial tazId)
{
		this->tazId = tazId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const TazLevelLandPrice& data)
        {
            return strm << "{"
                    << "\"tazId\":\"" << data.tazId << "\","
                    << "\"landValue\":\"" << data.landValue << "\","
                    << "}";
        }
    }
}





