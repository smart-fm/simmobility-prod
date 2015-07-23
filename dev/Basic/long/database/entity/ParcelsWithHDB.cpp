/*
 * ParcelsWithHDB.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: gishara
 */

#include "ParcelsWithHDB.hpp"

using namespace sim_mob::long_term;

ParcelsWithHDB::ParcelsWithHDB(BigSerial fmParcelId,int unitTypeId):
		fmParcelId(fmParcelId),unitTypeId(unitTypeId) {}

ParcelsWithHDB::~ParcelsWithHDB() {}

ParcelsWithHDB& ParcelsWithHDB::operator=(const ParcelsWithHDB& source)
{
	this->fmParcelId 			= source.fmParcelId;
	this->unitTypeId	= source.unitTypeId;
    return *this;
}

BigSerial ParcelsWithHDB::getFmParcelId() const
{
		return fmParcelId;
}

int ParcelsWithHDB::getUnitTypeId() const
{
		return unitTypeId;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const ParcelsWithHDB& data)
        {
            return strm << "{"
						<< "\"fmParcelId \":\"" << data.fmParcelId 	<< "\","
						<< "\"unitTypeId \":\"" 	<< data.unitTypeId 	<< "\","
						<< "}";
        }
    }
}





