/*
 * ParcelMatch.cpp
 *
 *  Created on: Aug 25, 2014
 *      Author: gishara
 */
#include "ParcelMatch.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

ParcelMatch::ParcelMatch(BigSerial fmParcelId, BigSerial slaParcelId,int matchCode,std::tm matchDate):
		fmParcelId(fmParcelId),slaParcelId(slaParcelId),matchCode(matchCode),matchDate(matchDate){
}

ParcelMatch::~ParcelMatch() {}

BigSerial ParcelMatch::getFmParcelId() const
{
    return fmParcelId;
}

BigSerial ParcelMatch::getSlaParcelId(BigSerial parcelId) const
{
	if(parcelId==this->fmParcelId)
	{
		return slaParcelId;
	}
    return 0;
}

BigSerial ParcelMatch::getSlaParcelId() const
{
    return slaParcelId;
}

int ParcelMatch::getMatchCode() const
{
	return matchCode;
}

std::tm ParcelMatch::getMatchDate() const
{
	return matchDate;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const ParcelMatch& data) {
            return strm << "{"
						<< "\"fm_parcel_id\":\"" << data.fmParcelId << "\","
						<< "\"sla_parcel_id\":\"" << data.slaParcelId << "\","
						<< "\"matchCode\":\"" << data.matchCode << "\","
						<< "\"matchDate\":\"" << data.matchDate.tm_year << "\","
						<< "}";
        }
    }
}



