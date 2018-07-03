/*
 * BuildingAvgAge.cpp
 *
 *  Created on: 23 Feb 2016
 *      Author: gishara
 */

#include "BuildingAvgAgePerParcel.hpp"

using namespace sim_mob::long_term;

BuildingAvgAgePerParcel::BuildingAvgAgePerParcel( BigSerial fmBParcelId, int age) : fmParcelId(fmParcelId), age(age){}

BuildingAvgAgePerParcel::~BuildingAvgAgePerParcel() {}

BuildingAvgAgePerParcel::BuildingAvgAgePerParcel( const BuildingAvgAgePerParcel &source)
{
	this->fmParcelId 			= source.fmParcelId;
	this->age = source.age;
}

BuildingAvgAgePerParcel& BuildingAvgAgePerParcel::operator=(const BuildingAvgAgePerParcel& source)
{
	this->fmParcelId 			= source.fmParcelId;
	this->age			= source.age;
    return *this;
}


BigSerial BuildingAvgAgePerParcel::getFmParcelId() const
{
	return fmParcelId;
}


int BuildingAvgAgePerParcel::getAge() const
{
	return age;
}

void BuildingAvgAgePerParcel::setFmParcelId(BigSerial parcelId) {
	this->fmParcelId = parcelId;
}

void BuildingAvgAgePerParcel::setAge(int age) {
	this->age = age;
}


namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const BuildingAvgAgePerParcel& data)
        {
            return strm << "{"
						<< "\"fm_building_id \":\"" << data.fmParcelId 	<< "\","
						<< "\"age \":\"" 	<< data.age 	<< "\","
						<< "}";
        }
    }
}



