/*
 * TotalBuildingSpace.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: gishara
 */

#include "TotalBuildingSpace.hpp"

using namespace sim_mob::long_term;

TotalBuildingSpace::TotalBuildingSpace( BigSerial fmParcelId,double totalBuildingSpace) :
					fmParcelId(fmParcelId), totalBuildingSpace(totalBuildingSpace){}

TotalBuildingSpace::~TotalBuildingSpace() {}

TotalBuildingSpace& TotalBuildingSpace::operator=(const TotalBuildingSpace& source)
{
	this->fmParcelId 			= source.fmParcelId;
	this->totalBuildingSpace	= source.totalBuildingSpace;
    return *this;
}


BigSerial TotalBuildingSpace::getFmParcelId() const
{
	return fmParcelId;
}

float TotalBuildingSpace::getTotalBuildingSpace() const
{
	return totalBuildingSpace;
}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const TotalBuildingSpace& data)
        {
            return strm << "{"
						<< "\"fm_parcel_id \":\"" << data.fmParcelId 	<< "\","
						<< "\"total_building_space \":\"" 	<< data.totalBuildingSpace 	<< "\","
						<< "}";
        }
    }
}



